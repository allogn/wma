/*
* 5 levels of debug is supported
* 0 : no any checking or stats at all
* 1 : input feasibility check
* 2 : advanced stats included
* 3 : debug info included
* 4 : everything else included
*/

#ifndef FCLA_FACILITYCHOOSER_H
#define FCLA_FACILITYCHOOSER_H

#include <forward_list>
#include <fstream>
#include "nheap.h"
#include "ExploringEdgeGenerator.h"
#include "TargetEdgeGenerator.h"
#include "Matcher.h"
#include "Network.h"
#include "Logger.h"
#include "helpers.h"

class FacilityChooser : public Matcher<long,long,long> {
public:
    enum State {
        UNINITIALIZED, NOT_LOCATED, LOCATED, UNFEASIBLE, ERROR
    };

    fHeap<long,long> heap; //one makes heap to have max first
    std::vector<long> result;
    long totalCost;
    long required_facilities;
    std::string exp_id;
    long facility_capacity;
    std::vector<long> source_indexes;
    State state = UNINITIALIZED;

    /*
     * lambda is a parameter that states when to terminate the heap exploration
     * it is equal to a minimal service filling required for considering that service
     */
    long lambda;

    /*
     * The matching bipartite graph has two node sets:
     * 1. customers (sources). Number of customers equal to size of source_node_index array
     * 2. services (targets, potential facility locations). Number of them is equal to size of a network
     */
    FacilityChooser(Network& network,
                    long facilities_to_locate,
                    long facility_capacity,
                    Logger* logger,
                    long lambda = 0) {
        this->exp_id = network.id;
        this->source_indexes = network.source_indexes;
        this->facility_capacity = facility_capacity;
        this->logger = logger;

        logger->add("number of facilities", facilities_to_locate);
        logger->add("capacity of facilities", facility_capacity);
        logger->add("lambda", lambda);

        this->heap.sign = 1; //make heap decreasing
        this->required_facilities = facilities_to_locate;
        this->lambda = lambda;

        //for Matching Algorithm (base class)
        graph_size = network.source_indexes.size() + igraph_vcount(&network.graph);
        logger->add("bipartite graph size", graph_size);

        igraph_empty(&graph, graph_size, true); //create directed graph
        this->node_excess.resize(graph_size,-1);
        for (long i = network.source_indexes.size(); i < graph_size; i++) {
            node_excess[i] = facility_capacity;
        }

#if _DEBUG_ > 0
        //should check here because no access to network in the rest of the code
        //(assuming facility chooser may not have been initialized on a network)
        logger->start("connectivity check time");
        igraph_bool_t check_connected;
        igraph_is_connected(&(network.graph),&check_connected,IGRAPH_WEAK);
        if (!check_connected) {
            //this is important because if one location should be placed - there should be a possibility of
            //checking the distance from each customer (creation of an edge in bipartite graph)
            logger->add("error", "Network must be weakly connected");
            this->state = UNFEASIBLE;
            return;
        }
        logger->finish("connectivity check time");
#endif

        this->edge_generator = new ExploringEdgeGenerator<long,long>(&(network.graph),
                                                                     network.weights,
                                                                     network.source_indexes);

        //reset variables of the matching algorithm
        reset();
        this->state = NOT_LOCATED;
    }

    ~FacilityChooser() {
        delete this->edge_generator;
    }

    //terminate if number of facilities exceed required_facilities
    bool greedySetCover(std::vector<std::forward_list<long>>& matchings) {
        //note that matching for target with vid=VID in the matching graph
        //will have VID-source_count index in matchings array because that one contains matchings only for targets
        std::vector<bool> covered(source_count, false);
        long total_covered = 0;
        while (heap.size() > 0) {
            long id, init_matching_count;
            heap.dequeue(id, init_matching_count);
            long only_target_id = id - source_count; //id in matchings, #of target node without source nodes
            long matching_count = 0;
            // check which matched vertex is still not covered and delete covered ones

            // linked list can delete only the NEXT element, so we are checking each next element,
            // and then the first one separately
            auto it = matchings[only_target_id].begin();
            auto prev_it = matchings[only_target_id].begin();
            //test if matching is not empty. If so - return false immediately
            if (it == matchings[only_target_id].end())
                return false;
            it++;
            while (it != matchings[only_target_id].end()) {
                long pair_id = (*it);
                if (covered[pair_id]) {
                    //delete from a linked-list
                    matchings[only_target_id].erase_after(prev_it);
                    it = prev_it;
                } else {
                    matching_count++;
                    prev_it = it;
                }
                it++;
            }
            //process the first one
            long first_node = (*matchings[only_target_id].begin());
            if (covered[first_node]) {
                matchings[only_target_id].pop_front();
            } else {
                matching_count++;
            }
            //if the size was changed - enheap back
            if (matching_count != init_matching_count) {
                heap.enqueue(id, matching_count);
            } else {
                // otherwise add to the result and update coverage
                result.push_back(only_target_id);
                if (result.size() > this->required_facilities) {
                    return false;
                }
                for (auto it = matchings[only_target_id].begin(); it != matchings[only_target_id].end(); it++) {
                    //every node in single-linked list must not be covered yet
                    covered[(*it)] = true;
                    total_covered++;
                }
                if (total_covered == this->source_count) {
                    return true;
                }
            }
        }
    }

    /*
     * Returns false if no set cover exists within current lambda
     */
    bool findSetCover() {
#if _DEBUG_ > 1
        logger->start("set cover check time");
#endif
        //initialize single linked lists and heaps
        result.clear();
        heap.clear();
        std::vector<std::forward_list<long>> matchings;
        // go through graph services and enheap them
        // according to the number of covered facilities = number of outgoing edges
        //for each service - put in a heap with a value = # of matched vertices
        igraph_vector_t eids;
        igraph_vector_init(&eids,0);
        for (long i = source_count; i < graph_size; i++) {
            forward_list<long> linked_nodes;
            igraph_incident(&graph, &eids, i, IGRAPH_OUT);
            //there can be many matched vertices because of capacities
            long matching_count = 0;
            for (igraph_integer_t i = 0; i < igraph_vector_size(&eids); i++) {
                igraph_integer_t eid = VECTOR(eids)[i];

                /* check if there is non-zero flow from a service to a customer
                 * if so - there is also a matching, otherwise skip.
                 * for example:
                 * init: A -> B (excess 3)
                 *       A <- B (excess 0 == full edge)
                 * after matching:
                 *       A -> B (excess 2)
                 *       A <- B (excess 1 => one unit was matched)
                 */
                if (edge_excess[eid] == 0) continue;
                igraph_integer_t target_node, source_node;
                igraph_edge(&graph,eid,&source_node,&target_node);
                linked_nodes.push_front(target_node);
                matching_count++;
            }
            //put in a heap
            if (matching_count >= lambda) {
                heap.enqueue(i, matching_count);
            }
            matchings.push_back(linked_nodes);
        }
        igraph_vector_destroy(&eids);
        //now run greedy algorithm, that chooses the best subset from a heap

#if _DEBUG_ > 1
        //logging outside function because of multiple returns inside
        logger->start("greedy set cover time");
#endif
        bool result = greedySetCover(matchings);

#if _DEBUG_ > 1
        logger->finish("greedy set cover time");
        logger->finish("set cover check time");
#endif

        return result;
    }

    void locateFacilities() {
        if (this->state != NOT_LOCATED) return; //probably infeasible because of initialization checks

        if (this->required_facilities*this->facility_capacity < this->edge_generator->n) {
            logger->add("error", "Problem infeasible: not enough facilities");
            this->state = UNFEASIBLE;
            return;
        }

        logger->start("runtime");

        //calculate preliminary matching
        logger->start("prematching time");
        this->run();
        logger->finish("prematching time");

        // increase customer capacities until we can choose a covering subset of matched services
        long capacity_iteration = 0;
        while (!this->findSetCover()) {
            capacity_iteration++;
            //increase capacities of everyone
#if _DEBUG_ > 1
            logger->start("increase capacity time");
#endif
            for (long vid = 0; vid < source_count; vid++) {
                this->increaseCapacity(vid);
            }
#if _DEBUG_ > 1
            logger->finish("increase capacity time");
#endif
        }
        logger->add("number of iterations", capacity_iteration);

        /*
         * we place "free" facilities right near the customers with "farthest" matching
         * so in calculateResult function we must recalculate distances
         */
        logger->add("facilities left after termination", (long)(required_facilities - this->result.size()));
        logger->start("lack facility allocation time");
        if (this->result.size() < required_facilities) {
            //find the total number of available facilities
            long facilities_left = required_facilities - this->result.size();
            if (this->source_count <= facilities_left) {
                //trivial solution - objective, objective = 0, place facilities in customer locations
                for (long i = 0; i < this->source_count; i++) {
                    this->result.push_back(this->source_indexes[i]);
                }
                for (long i = 0; i < facilities_left - this->source_count; i++) {
                    this->result.push_back(this->source_indexes[0]);
                }

                logger->finish("lack facility allocation time");
                logger->finish("runtime");
                this->state = LOCATED;
                return;
            }
            //get worst <facilities_left> covered customers by existing result
            //go throught all customers and for each find the smallest (best) option
            //then choose top-<facilities_left> options
            std::vector<long> source_best(this->source_count, LONG_MAX);
            for (long i = 0; i < this->result.size(); i++) {
                //each element in result array is a facility location
                //bipartite graph still holds customers that were matched to that location
                //note that one customer can be matched with several facilities
                //so for current facility location we get all covered customers, add (or update!) them in the heap
                long location_vid = this->result[i] + this->source_count; //result contains ids in a network graph
                //traverse each matched customer in the bipartite graph
                igraph_vector_t eids;
                igraph_vector_init(&eids, 0);
                igraph_incident(&graph, &eids, location_vid, IGRAPH_OUT);
                for (igraph_integer_t i = 0; i < igraph_vector_size(&eids); i++) {
                    igraph_integer_t eid = VECTOR(eids)[i];
                    if (edge_excess[eid] == 0) continue; //no matching
                    igraph_integer_t target_node, source_node; //target node is a left(source) node in bipartite graph
                    igraph_edge(&graph,eid,&source_node,&target_node);
                    //target node now is vid of a customer and |weights of edge| is the distance
                    long cur_dist = -this->weights[eid]; //minus because the edge is inverted
                    source_best[target_node] = std::min(source_best[target_node], cur_dist);
                }
                igraph_vector_destroy(&eids);
            }
            //now get top-k and add to the result
            fHeap<long,long> partialHeapsort;
            for (long i = 0; i < this->source_count; i++) {
                if (partialHeapsort.size() < facilities_left) {
                    partialHeapsort.enqueue(i, source_best[i]);
                } else {
                    if (partialHeapsort.getTopValue() < source_best[i]) { //we want to get top worst, i.e. top biggest
                        partialHeapsort.dequeue();
                        partialHeapsort.enqueue(i, source_best[i]);
                    }
                }
            }
            long new_location;
            while (partialHeapsort.dequeue(new_location)) {
                this->result.push_back(new_location-this->source_count); //put location in a network
            }
        }
        logger->finish("lack facility allocation time");
        logger->finish("runtime");
        this->state = LOCATED;
    }

    long calculateResult() {
        //calculate Total Sum
        if (this->state == UNFEASIBLE) {
            this->totalCost = -1;
            return -1;
        }
        if (this->state != LOCATED) {
            throw "Wrong class state: calculate locations first.";
        }
        logger->start("result final calculation time");

        //run matching in resulting biparite graph, but having capacity of 1 only and having customers
        //on the right side of bipartite graph

        //build valid bipartite graph : reverse all edges to the direct position
        //create an edge generator with calculated distances between known facilities and customers
        //run new matcher
        std::vector<long> new_excess(this->source_indexes.size() + this->result.size(),this->facility_capacity);
        for (long i = 0; i < this->source_indexes.size(); i++) {
            new_excess[i] = -1;
        }
        TargetEdgeGenerator short_bigraph_generator(this, this->result);
        Matcher<long,long,long> M(&short_bigraph_generator, new_excess, this->logger);
        M.run();
        M.calculateResult();
        this->totalCost = M.result_weight;

        logger->finish("result final calculation time");
        logger->add("objective", totalCost);
        return totalCost;
    }
};

#endif //FCLA_FACILITYCHOOSER_H
