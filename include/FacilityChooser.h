/*
* 5 levels of debug is supported
* 0 : no any checking or stats at all
* 1 : input feasibility check and basic stats
* 2 : advanced stats included
* 3 : debug info included
* 4 : everything else included
*/

#ifndef FCLA_FACILITYCHOOSER_H
#define FCLA_FACILITYCHOOSER_H

#include <forward_list>
#include <fstream>
#include <stack>
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

    FacilityChooser() {}; //for testing

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
        logger->start2("fcla initialization");
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

#if _DEBUG_ > 0
        //should check here because no access to network in the rest of the code
        //(assuming facility chooser may not have been initialized on a network)
        logger->start1("connectivity check time");
        igraph_bool_t check_connected;
        igraph_is_connected(&(network.graph),&check_connected,IGRAPH_WEAK);
        if (!check_connected) {
            //this is important because if one location should be placed - there should be a possibility of
            //checking the distance from each customer (creation of an edge in bipartite graph)
            logger->add("error", "Network must be weakly connected");
            this->state = UNFEASIBLE;
            return;
        }
        logger->finish1("connectivity check time");
#endif

        this->edge_generator = new ExploringEdgeGenerator<long,long>(&(network.graph),
                                                                     network.weights,
                                                                     network.source_indexes);
        graph_size = edge_generator->n + edge_generator->m;
        source_count = edge_generator->n;

        //for Matching Algorithm (base class)
        logger->add2("bipartite graph size", graph_size);

        this->node_excess.resize(graph_size,-1);
        for (long i = network.source_indexes.size(); i < graph_size; i++) {
            node_excess[i] = facility_capacity;
        }

        //reset variables of the matching algorithm
        reset();
        this->state = NOT_LOCATED;
        logger->finish("fcla initialization");
    }

    ~FacilityChooser() {
        delete this->edge_generator;
    }

    //terminate if number of facilities exceed required_facilities
    bool greedySetCover(std::vector<std::forward_list<long>>& matchings) {
        //note that matching for target with vid=VID in the matching graph
        //will have VID-source_count index in matchings array because that one contains matchings only for targets
        std::vector<bool> covered(this->source_count, false);
        long total_covered = 0;
        while (heap.size() > 0) {
            long id, init_matching_count;
            heap.dequeue(id, init_matching_count);
            long only_target_id = id - this->source_count; //id in matchings, #of target node without source nodes
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
                if (total_covered == source_count) {
                    return true;
                }
            }
        }
        throw "Something is wrong";
        return false; //never should reach this
    }

    /*
     * Fulfills result with a set of facilities that cover all customers or returns false if does not exist
     *
     * Idea: vector of counters for each customer - if covered. Then take a set of facilities (maybe ordered) //@todo check if ordered by overlapping measure is better?
     * And in a tree traversal way try to exclude up to F'-F facilities until a valid set is found
     */
    bool treeCover(std::vector<long>& facility_candidates) {
        std::vector<long> customer_cover(this->source_count,0);
        //get a set of top-lambda facilities, each represents a set of covered customers
        //populare cover vector with all candidates
        for (long i = 0; i < facility_candidates.size(); i++) {
            long facility_id = facility_candidates[i];
            //iterate through all customers covered by this facility
            for (EdgeIterator it = edges[facility_id].begin(); it != edges[facility_id].end(); it++) {
                customer_cover[it->first]++;
            }
        }

        for (long i = 0; i < customer_cover.size(); i++) {
            if (customer_cover[i] == 0) return false;
        }

        std::stack<long> index_stack; //each index is a facility index in facility_candidates array
        index_stack.push(0);
        while (index_stack.size() > 0) {
            long current_index = index_stack.top();
            long facility_id = facility_candidates[current_index];
            bool valid = true;

            //substract covered customers from coverage vector
            for (EdgeIterator it = edges[facility_id].begin(); it != edges[facility_id].end(); it++) {
                customer_cover[it->first]--;
                //if any of touched customers is now zero : invalidate current index by removing it from stack
                if (customer_cover[it->first] == 0) {
                    valid = false;
                    //do not break here because we need to increase back coverage later
                }
            }

            //now we have either valid current index on the top of the stack or ancestor on the top
            //that is not equal to current_index

            //if we have valid current index, then we should check if we eliminated enough facilities
            //number of eliminated facilities is equal to the size of stack because the last element is valid
            //(remain all customers covered)
            if (valid && (index_stack.size() == facility_candidates.size() - this->required_facilities)) {
                //solution found
                break;
            }

            /*
             * check whether we have enough facilities in the subtree by this equation:
             * (facility_candidates.size() - this->required_facilities) - index_stack.size() : left facilities to place
             * facility_candidates.size() - current_index - 1 : size of subtree
             * size of subtree < left facilities to place | *(-1) => invalidate
             */
            if (valid && (this->required_facilities + index_stack.size() < current_index + 1)) {
                valid = false; //too few facilities left
            }

            //if there is no subtree left, but we still haven't eliminated enough facilities, then we are at the dead brunch
            //we should pop current valid index and enheap next one instead of it (if any)

            //if we don't have valid top element, then we are at the dead brunch
            if (!valid) {
                index_stack.pop();
                //if we pop - then we increase back the coverage (for current candidate)
                for (EdgeIterator it = edges[facility_id].begin(); it != edges[facility_id].end(); it++) {
                    customer_cover[it->first]++;
                }
            }
            //in any case we try to push (independently if we are valid or invalid)
            if (current_index < facility_candidates.size() - 1) { //there is room for increasing

                //if we have valid top element, but it is not an answer yet, then we start traversing subtree
                //we initialize traversing by putting next index to the stack
                //at the same time we do it only if there is any subtree

                index_stack.push(current_index + 1);
            }
        }

        this->result = facility_candidates;
        //our solution is in the stack
        //stack contains facilities that should be DELETED
        //if stack is empty, then nothing can be deleted
        if (index_stack.size() == 0) return true;

        while (index_stack.size() > 0) {
            //delete every result
            long index_to_delete = index_stack.top();
            this->result.erase(this->result.begin() + index_to_delete);
            index_stack.pop();
        }
        return true;
    }

    /*
     * Returns false if no set cover exists within current lambda
     */
    bool findSetCover() {
        logger->start2("set cover check time");
        //initialize single linked lists and heaps
        result.clear();
        heap.clear();
        std::vector<std::forward_list<long>> matchings;
        // go through graph services and enheap them
        // according to the number of covered facilities = number of outgoing edges
        //for each service - put in a heap with a value = # of matched vertices
        for (long i = source_count; i < graph_size; i++) {
            forward_list<long> linked_nodes;
            //there can be many matched vertices because of capacities
            long matching_count = 0;
            EdgeIterator it;
            for (it = edges[i].begin(); it != edges[i].end(); it++) {
                linked_nodes.push_front(it->first);
                matching_count++;
            }
            //put in a heap
            if (matching_count >= lambda) {
                heap.enqueue(i, matching_count);
            }
            matchings.push_back(linked_nodes);
        }
        //now run greedy algorithm, that chooses the best subset from a heap

        //logging outside function because of multiple returns inside
        logger->start2("greedy set cover time");
        bool if_result = greedySetCover(matchings);
        logger->finish2("greedy set cover time");
        logger->finish2("set cover check time");

        return if_result;
    }

    void locateFacilities() {
        if (this->state != NOT_LOCATED) return; //probably infeasible because of initialization checks

        if (this->required_facilities*this->facility_capacity < source_count) {
            logger->add("error", "Problem infeasible: not enough facilities");
            this->state = UNFEASIBLE;
            return;
        }

        logger->start("runtime");

        //calculate preliminary matching
        logger->start2("prematching time");
        this->run();
        logger->finish2("prematching time");

        // increase customer capacities until we can choose a covering subset of matched services
        long capacity_iteration = 0;
        while (!this->findSetCover()) {
            capacity_iteration++;
            //increase capacities of everyone
            logger->start2("increase capacity time");
            for (long vid = 0; vid < source_count; vid++) {
                this->increaseCapacity(vid);
            }
            logger->finish2("increase capacity time");
        }
        logger->add1("number of iterations", capacity_iteration);

        /*
         * we place "free" facilities right near the customers with "farthest" matching
         * so in calculateResult function we must recalculate distances
         */
        logger->add1("facilities left after termination", (long)(required_facilities - this->result.size()));
        logger->start2("lack facility allocation time");
        if (this->result.size() < required_facilities) {
            //find the total number of available facilities
            long facilities_left = required_facilities - this->result.size();
            if (this->source_count <= facilities_left) {
                //trivial solution - objective, objective = 0, place facilities in customer locations
                for (long i = 0; i < source_count; i++) {
                    this->result.push_back(this->source_indexes[i]);
                }
                for (long i = 0; i < facilities_left - source_count; i++) {
                    this->result.push_back(this->source_indexes[0]);
                }

                logger->finish2("lack facility allocation time");
                logger->finish("runtime");
                this->state = LOCATED;
                return;
            }
            //get worst <facilities_left> covered customers by existing result
            //go through all customers and for each find the smallest (best) option
            //then choose top-<facilities_left> options
            std::vector<long> source_best(source_count, LONG_MAX);
            for (long i = 0; i < this->result.size(); i++) {
                //each element in result array is a facility location
                //bipartite graph still holds customers that were matched to that location
                //note that one customer can be matched with several facilities
                //so for current facility location we get all covered customers, add (or update!) them in the heap
                long location_vid = this->result[i] + source_count; //result contains ids in a network graph
                //traverse each matched customer in the bipartite graph
                for (EdgeIterator it = edges[location_vid].begin(); it != edges[location_vid].end(); it++) {
                    long cur_dist = -it->second; //minus because the edge is inverted
                    source_best[it->first] = std::min(source_best[it->first], cur_dist);
                }
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
        logger->finish2("lack facility allocation time");
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
