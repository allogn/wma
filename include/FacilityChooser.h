#ifndef FCLA_FACILITYCHOOSER_H
#define FCLA_FACILITYCHOOSER_H

#include <forward_list>
#include <fstream>
#include "nheap.h"
#include "ExploringEdgeGenerator.h"
#include "Matcher.h"
#include "Network.h"

class FacilityChooser : public Matcher<long,long,long> {
public:
    fHeap<long,long> heap; //one makes heap to have max first
    std::vector<long> result;
    long totalCost;
    long required_facilities;
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
                    long lambda = 0) {
        igraph_bool_t check_connected;
        igraph_is_connected(&(network.graph),&check_connected,IGRAPH_WEAK);
        if (!check_connected) {
            //this is important because if one location should be placed - there should be a possibility of
            //checking the distance from each customer (creation of an edge in bipartite graph)
            throw std::string("Network must be weakly connected");
        }
        if (facilities_to_locate*facility_capacity < network.source_indexes.size()) {
            throw std::string("Problem infeasible: not enough facilities");
        }

        heap.sign = 1; //make heap decreasing

        this->required_facilities = facilities_to_locate;
        this->lambda = lambda;

        //for Matching Algorithm (base class)
        graph_size = network.source_indexes.size() + igraph_vcount(&network.graph);
        igraph_empty(&graph, graph_size, true); //create directed graph
        this->node_excess.resize(graph_size,-1);
        for (long i = network.source_indexes.size(); i < graph_size; i++) {
            node_excess[i] = facility_capacity;
        }

        this->edge_generator = new ExploringEdgeGenerator<long,long>(&(network.graph),
                                                                     network.weights,
                                                                     network.source_indexes);

        //reset variables of the matching algorithm
        reset();
    }

    FacilityChooser(igraph_t* network,
                    std::vector<long>& weights,
                    std::vector<long>& source_node_index,
                    long facilities_to_locate,
                    long facility_capacity,
                    long lambda = 0)
    {
        heap.sign = 1; //make heap decreasing

        this->required_facilities = facilities_to_locate;
        this->lambda = lambda;

        //for Matching Algorithm (base class)
        graph_size = source_node_index.size() + igraph_vcount(network);
        igraph_empty(&graph, graph_size, true); //create directed graph
        this->node_excess.resize(graph_size,-1);
        for (long i = source_node_index.size(); i < graph_size; i++) {
            node_excess[i] = facility_capacity;
        }

        this->edge_generator = new ExploringEdgeGenerator<long,long>(network, weights, source_node_index);

        //reset variables of the matching algorithm
        reset();
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
                 * for exmpale:
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
        return greedySetCover(matchings);
    }

    void locateFacilities() {
        this->run(); //calculate preliminary matching

        // increase customer capacities until we can choose a covering subset of matched services
        while (!this->findSetCover()) {
            //increase capacities of everyone
            for (long vid = 0; vid < source_count; vid++) {
                this->increaseCapacity(vid);
            }
        }
        /*
         * we place "free" facilities right near the customers with "farthest" matching
         * so in calculateResult function we must recalculate distances
         */
        if (this->result.size() < required_facilities) {
            //find the total number of available facilities
            long facilities_left = required_facilities - this->result.size();
            //get worst <facilities_left> covered customers by existing result
            fHeap<long, long> topHeap; //min heap (top is the smallest)
            for (long i = 0; i < this->result.size(); i++) {
                //each element in result array is a facility location
                //bipartite graph still holds customers that were matched to that location
                //note that one customer can be matched with several facilities
                //so for current facility location we get all covered customers, add (or update!) them in the heap
                long location_vid = this->result[i] + this->source_count; //result contains ids in a network graph
                //traverse each matched customer in the bipartite graph
                igraph_vector_t eids;
                igraph_vector_init(&eids, 0);
                igraph_incident(&graph, &eids, i, IGRAPH_OUT);
                for (igraph_integer_t i = 0; i < igraph_vector_size(&eids); i++) {
                    igraph_integer_t eid = VECTOR(eids)[i];
                    if (edge_excess[eid] == 0) continue; //no matching
                    igraph_integer_t target_node, source_node;
                    igraph_edge(&graph,eid,&source_node,&target_node);
                    //target node now is vid of a customer and |weights of edge| is the distance
                    long cur_dist = -this->weights[eid]; //minus because the edge is inverted
                    if ((topHeap.size() < facilities_left) || (topHeap.getTopValue() < cur_dist)) {
                        //enqueue or update: we are looking for the top biggest (note that heap in min-heap still)
                        //so we update only if the value in the heap is smaller
                        long heaped_val;
                        if (topHeap.isExisted(target_node, heaped_val)) {
                            if (heaped_val < cur_dist) {
                                topHeap.updatequeue(target_node, cur_dist);
                            }
                        } else {
                            //remove the smallest element and enqueue the new one
                            topHeap.dequeue();
                            topHeap.enqueue(target_node, cur_dist);
                        }
                    }
                }
                igraph_vector_destroy(&eids);
            }
            //now topHeap contains the best customers
            if (topHeap.size() < facilities_left) {
                throw std::string("The problem is trivial, too many facilities: "
                        "place one facility near each customer, and some facilities are still left");
            }
            long new_location;
            while (topHeap.dequeue(new_location)) {
                this->result.push_back(new_location-this->source_count); //put location in a network
            }
        }
    }

    long calculateResult() {
        //calculate Total Sum
        if (this->result.size() != this->required_facilities) {
            throw std::string("Facilities are not located");
        }
        totalCost = 0;
        for (long i = 0; i < this->source_count; i++) {
            //find closest facility for each customer
            long closest_dist = LONG_MAX;
            for (long j = 0; j < this->result.size(); j++) {
                long bipart_facility_vid = this->result[j] + this->source_count;
                igraph_integer_t eid;
                //note here, that eid might not be presented in bipartite graph
                //if a facility is too far from a customer which is not the target one for this facility
                //so we assume in this case that this facility is not the closest for a customer,
                //but then the result may have an error here and should be tested for correctness @todo
                igraph_get_eid(&graph, &eid, i, bipart_facility_vid, true, false);
                if (eid != -1) {
                    closest_dist = min(closest_dist, this->weights[eid]);
                }
            }
            totalCost += closest_dist;
        }
        return totalCost;
    }

    void saveResult(std::string filename) {
        std::ofstream f(filename, std::ios::out);
        f << totalCost << "\n";
        for (long i = 0; i < this->result.size(); i++) {
            f << this->result[i] << "\n";
        }
        f.close();
    }
};

#endif //FCLA_FACILITYCHOOSER_H
