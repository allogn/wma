#ifndef FCLA_FACILITYCHOOSER_H
#define FCLA_FACILITYCHOOSER_H

#include <forward_list>
#include "nheap.h"
#include "ExploringEdgeGenerator.h"
#include "Matcher.h"

class FacilityChooser : public Matcher<long,long,long> {
public:
    fHeap<long,long> heap; //one makes heap to have max first
    std::vector<long> result;
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

        this->edge_generator = new ExploringEdgeGenerator<long,long>(network,
                                                                     weights,
                                                                     source_node_index,
                                                                     facility_capacity);

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
    }
};

#endif //FCLA_FACILITYCHOOSER_H
