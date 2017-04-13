/*
* 5 levels of debug is supported
* 0 : no any checking or stats at all
* 1 : input feasibility check and basic stats
* 2 : advanced stats included (nested)
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
// _define_ var inherited here
#include "Logger.h"
#include "helpers.h"

class FacilityChooser : public Matcher<long,long,long> {
public:
    enum State {
        UNINITIALIZED, NOT_LOCATED, LOCATED, UNFEASIBLE, ERROR
    };

    std::vector<long> result;
    long totalCost;
    long required_facilities;
    std::string exp_id;
    long facility_capacity;
    std::vector<long> source_indexes;
    State state = UNINITIALIZED;
    std::vector<long> customer_antirank;
    double alpha;
    double gamma;

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
                    long lambda = 0,
                    double alpha = 1,
                    double gamma = 0) {
        logger->start2("fcla initialization");
        this->exp_id = network.id;
        this->source_indexes = network.source_indexes;
        this->facility_capacity = facility_capacity;
        this->logger = logger;
        this->alpha = alpha;
        this->gamma = gamma;

        logger->add("number of facilities", facilities_to_locate);
        logger->add("capacity of facilities", facility_capacity);
        logger->add("lambda", lambda);

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
        this->customer_antirank.clear();
        this->customer_antirank.resize(source_count);

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
    //note that vector should be copied each time because it is modified internally, but we don't use them in brute force anymore if greedy is called
    //so they are copied implicitly in brute force (each brute force has local version)
    bool greedySetCover(std::vector<std::forward_list<long>>& matchings, std::vector<long>& local_covered,
                        long& total_covered, fHeap<long,long>& heap, std::vector<long>& result) {
        //note that matching for target with vid=VID in the matching graph
        //will have VID-source_count index in matchings array because that one contains matchings only for targets

        //logging variables
        long heap_iterations = 0;

        while (heap.size() > 0) {
            heap_iterations++;
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
            if (it == matchings[only_target_id].end()) {
                logger->add2("greedy deheap iterations", heap_iterations);
                return false;
            }
            it++;
            while (it != matchings[only_target_id].end()) {
                long pair_id = (*it);
                if (local_covered[pair_id]) {
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
            if (local_covered[first_node]) {
                matchings[only_target_id].pop_front();
            } else {
                matching_count++;
            }

            //if the size was changed - enheap back
            if (matching_count != init_matching_count) {
                //not guaranteed to have a non-decreasing matching count
                heap.enqueue(id, matching_count);
            } else {

                double relative_gain = (double) matching_count / (double) (source_count - total_covered);
                logger->add2("relative gain", relative_gain);
                logger->add2("matching count", matching_count);
                logger->add2("left count", source_count - total_covered);
                // otherwise add to the result and update coverage
                result.push_back(only_target_id);

                if (result.size() > this->required_facilities + lambda) {
                    logger->add2("greedy deheap iterations", heap_iterations);
                    return false;
                }

                //relative gain threshold
//                if (((result.size() > this->required_facilities) && ((double)(source_count - total_covered)/(double)source_count > 0.1)) ||
//                        ((result.size() > this->required_facilities + lambda) && ((double)(source_count - total_covered)/(double)source_count <= 0.1))) {
//                    logger->add2("greedy deheap iterations", heap_iterations);
//                    return false;
//                }

                for (auto it = matchings[only_target_id].begin(); it != matchings[only_target_id].end(); it++) {
                    //every node in single-linked list must not be covered yet
                    local_covered[(*it)]++;
                    total_covered++;
                }

                //if number of covered is not enough guaranteed - lambda case
//                if (matching_count * (this->required_facilities - this->result.size() + lambda) < source_count - total_covered) {
//                    logger->add2("greedy deheap iterations", heap_iterations);
//                    return false;
//                }


                if (total_covered == source_count) {
                    logger->add2("greedy deheap iterations", heap_iterations);
                    return true;
                }
            }
        }
        throw "Something is wrong";
        return false; //never should reach this
    }

    //terminate if number of facilities exceed required_facilities
//    bool greedySetCoverNoHeap() {
//        //note that matching for target with vid=VID in the matching graph
//        //will have VID-source_count index in matchings array because that one contains matchings only for targets
//        long total_covered = 0;
//        while (result.size() <= this->required_facilities) {
//            long best_fac_id = 0;
//            long best_fac_coverage = -1;
//            for (long id = 0; id < this->required_facilities; id++) {
//                long matching_count = 0;
//                // check which matched vertex is still not covered and delete covered ones
//
//                // linked list can delete only the NEXT element, so we are checking each next element,
//                // and then the first one separately
//                long uncovered = 0;
//                for (EdgeIterator it = edges[id + this->source_count].begin();
//                     it != edges[id + this->source_count].end(); it++) {
//                    long pair_id = it->first;
//                    if (!covered[pair_id]) {
//                        uncovered++;
//                    }
//                }
//                if (uncovered > best_fac_coverage) {
//                    best_fac_coverage = uncovered;
//                    best_fac_id = id;
//                }
//            }
//
//            double relative_gain = (double) best_fac_coverage / (double) (source_count - total_covered);
//            logger->add2("relative gain", relative_gain);
//            logger->add2("matching count", best_fac_coverage);
//            logger->add2("left count", source_count - total_covered);
//            // otherwise add to the result and update coverage
//            result.push_back(best_fac_id);
//            for (EdgeIterator it = edges[best_fac_id + this->source_count].begin();
//                 it != edges[best_fac_id + this->source_count].end(); it++) {
//                covered[it->first] = true;
//            }
//            total_covered += best_fac_coverage;
//            if (total_covered == source_count) {
//                return true;
//            }
//        }
//        return false;
//    }

    /*
     * Fulfills result with a set of facilities that cover all customers or returns false if does not exist
     *
     * Idea: vector of counters for each customer - if covered. Then take a set of facilities (maybe ordered) //@todo check if ordered by overlapping measure is better?
     * And in a tree traversal way try to exclude up to F'-F facilities until a valid set is found
     */
    bool treeCover(std::vector<long>& facility_candidates) {
        //note that facility candidates  Is a result vector that holds indices in a network but not this bipartite graph
        std::vector<long> customer_cover(this->source_count,0);
        //get a set of top-lambda facilities, each represents a set of covered customers
        //populare cover vector with all candidates
        for (long i = 0; i < facility_candidates.size(); i++) {
            long facility_vid = facility_candidates[i] + this->source_count;
            //iterate through all customers covered by this facility
            for (EdgeIterator it = edges[facility_vid].begin(); it != edges[facility_vid].end(); it++) {
                customer_cover[it->first]++;
            }
        }

        for (long i = 0; i < customer_cover.size(); i++) {
            if (customer_cover[i] == 0) return false;
        }
        if (facility_candidates.size() <= this->required_facilities) {
            return true;
        }

        std::stack<long> index_stack; //each index is a facility index in facility_candidates array
        index_stack.push(0);
        bool solution_exist = false;

        while (index_stack.size() > 0) {
            long current_index = index_stack.top();
            long facility_vid = facility_candidates[current_index] + this->source_count;
            bool valid = true;

            //substract covered customers from coverage vector
            for (EdgeIterator it = edges[facility_vid].begin(); it != edges[facility_vid].end(); it++) {
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
            if (valid && (facility_candidates.size() - index_stack.size() <= this->required_facilities)) {
                //solution found
                solution_exist = true;
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
                for (EdgeIterator it = edges[facility_vid].begin(); it != edges[facility_vid].end(); it++) {
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

        /*
         * All covered, but nothing to delete
         */
        if (!solution_exist) return false;

        //our solution is in the stack
        //stack contains facilities that should be DELETED
        //if stack is empty, then nothing can be deleted
        if (index_stack.size() == 0) return true;

        while (index_stack.size() > 0) {
            //delete every result
            long index_to_delete = index_stack.top();
            facility_candidates.erase(facility_candidates.begin() + index_to_delete);
            index_stack.pop();
        }
        return true;
    }

    /*
     * Check for a level of coverage in a heap whether there is a coverage
     * All parameters are copied to a local variables (passed not by reference)
     *
     */
    //heap passed by reference because a copy is created inside while loop (to reuse initial state for each facility set)
    //matchings the same
    bool proceedLevel(std::vector<long>& covered, long& total_covered, fHeap<long,long>& heap,
                      std::vector<std::forward_list<long>>& matchings, std::vector<long>& result) {
        if (heap.size() == 0) {
            return false; //out of available facilities for some reason (probably border case)
        }
        long current_coverage = heap.getTopValue();

        //we can check strong feasibility right here
        //coverage is not feasible if marginal gain for remaining facilities is not enough
        //this includes the extreme case when no available facilities left
        if (current_coverage*(this->required_facilities - result.size()) < (this->source_count - total_covered)) {
            return false;
        }

        if (source_count == total_covered) {
            this->result = result;
            return true;
        }

        //now check if we need brute force
        if (this->facility_capacity - current_coverage >= gamma) {
            logger->add2("proceedLevelType",0);
            //now run greedy algorithm, that chooses the best subset from a heap for current delta_covered set
            logger->start2("greedy set cover time");
            bool if_result = greedySetCover(matchings, covered, total_covered, heap, result); //result modified inside
            logger->finish2("greedy set cover time");

            logger->start2("tree set cover time");
            if (if_result && (result.size() > this->required_facilities)) { //in case lambda > 0
                //try set cover
                if_result = treeCover(result); //result vector updated if success
            }
            logger->finish2("tree set cover time");
            this->result = result;

            for (long i = 0; i < source_count; i++) {
                this->customer_antirank[i] += (covered[i] == 0);
            }

            return if_result;
        }
        logger->add2("proceedLevelType",1);

        //deheap all of current level
        long cur_coverage_facility_count = 0;
        std::forward_list<long> current_level_facilities;
        while (heap.size() > 0 && heap.getTopValue() == current_coverage) {
            current_level_facilities.push_front(heap.getTopIdx());
            heap.dequeue();
            cur_coverage_facility_count++;
        }
        logger->add2("current level fac count", cur_coverage_facility_count);

        /*
         * the idea of taking "left" amount of facilities is example of remaining only 1 uncovered customer,
         * then we must bruteforce all facilities up to the capacity 1
         *
         * cur_coverage_facility_count is number of facilities with current coverage
         * this->source_count - total_covered = total uncovered facilities
         *
         * when number of facilities with current coverage is enough to cover all uncovered yet customers (extreme)
         * multiplied by gamma that weakens this requirement
         */

        //for each A in a set of current level
        //note: A might have coverage not equal to current_coverage. example: if we start with not first level.
        //we want to iterate through independent sets with current_coverage, so we reenheap starting facilities
        //until we find the good one for creating the independent set
        long best_covered = total_covered;
        while (current_level_facilities.begin() != current_level_facilities.end()) {
            std::vector<long> new_covered = covered; //not logged because this should be before coverage checking
                                                     //others only after current candidate
            long delta_total_covered = 0;

            long facility_id = *current_level_facilities.begin() - source_count; //this is heap idx, that is build by id in bipartite
            current_level_facilities.pop_front();
            //matchings may hold used customers, but we calculate integer delta
            for (std::forward_list<long>::iterator it = matchings[facility_id].begin(); it != matchings[facility_id].end(); it++) {
                delta_total_covered += (new_covered[*it] == 0);
                new_covered[*it]++;
            }
            //if delta_total_covered is not equal to promised current_coverage (fall short of expectations), then enheap and continue
            if (delta_total_covered != current_coverage) {
                heap.enqueue(facility_id + source_count, delta_total_covered);
                continue;
            }

            //otherwise add to result and start building independent set
            //initialize new variables
            logger->start2("brute-force copying");
            fHeap<long,long> new_heap = heap;
            std::vector<std::forward_list<long>> new_matchings = matchings;
            std::vector<long> new_result = result;
            logger->finish2("brute-force copying");

            new_matchings[facility_id].clear();
            new_result.push_back(facility_id);

            //check if we are done by this level
            if (total_covered+delta_total_covered == source_count) {
                //we are done by adding independent facilities at this level
                this->result = new_result;
                return true;
            }

            //for each B else in the set
            std::forward_list<long>::iterator fac_it = current_level_facilities.begin();
            std::forward_list<long>::iterator prev_fac_it = fac_it;
            while (fac_it != current_level_facilities.end()) {
                long another_facility_id = *fac_it - source_count;
                //if A influenced B then enheap back
                long new_coverage = 0;
                bool interfere = false;
                for (std::forward_list<long>::iterator it = new_matchings[another_facility_id].begin();
                     it != new_matchings[another_facility_id].end(); it++) {
                    if (new_covered[*it] == 0) {
                        new_coverage++;
                    } else {
                        interfere = true;
                    }
                }

                if (interfere) {
                    //update new_matchings, delete already covered customers
                    auto it = new_matchings[another_facility_id].begin();
                    auto prev_it = it++;
                    while (it != new_matchings[another_facility_id].end()) {
                        if (new_covered[*it] > 0) {
                            new_matchings[another_facility_id].erase_after(prev_it);
                            it = prev_it;
                        } else {
                            prev_it = it;
                        }
                        it++;
                    }
                    it = new_matchings[another_facility_id].begin();
                    if (new_covered[*it] > 0) {
                        new_matchings[another_facility_id].pop_front();
                    }

                    new_heap.enqueue(another_facility_id + source_count, new_coverage);
                } else {
                    //else remove B from the set of current level (do not consider it), and add one more facility to current delta_coverage
                    for (std::forward_list<long>::iterator it = new_matchings[another_facility_id].begin();
                         it != new_matchings[another_facility_id].end(); it++) {
                        new_covered[*it]++;
                    }
                    new_matchings[another_facility_id].clear();
                    delta_total_covered += new_coverage;
                    new_result.push_back(another_facility_id);

                    if (total_covered+delta_total_covered == source_count) {
                        //we are done by adding independent facilities at this level
                        this->result = new_result;
                        return true;
                    }

                    /*
                     * we do not check here whether our result is too big, because we imply that all facilities
                     * in the current independent set covered the same current_coverage customers, and we checked
                     * the assumption if every facility left has such coverage then we are done succesfully
                     */

                    //remove B
                    if (prev_fac_it == fac_it) {
                        //we are at the first node, so just delete it
                        current_level_facilities.pop_front();
                        fac_it = current_level_facilities.begin();
                    } else {
                        current_level_facilities.erase_after(prev_fac_it);
                        fac_it = prev_fac_it;
                    }
                }

                prev_fac_it = fac_it;
                if (current_level_facilities.begin() != current_level_facilities.end()) fac_it++;
            }
            long new_total_covered = total_covered + delta_total_covered;
            bool if_result = proceedLevel(new_covered, new_total_covered, new_heap,
                                          new_matchings, new_result);
            best_covered = (best_covered > new_total_covered) ? best_covered : new_total_covered;
            if (if_result) {
                return true; //done
            } //else - check other possibilities
        }
        total_covered = best_covered;
        return false; //all possibilities checked
    }


    /*
     * Returns false if no set cover exists within current lambda
     */
    bool findSetCover() {
        logger->start2("set cover check time");
        //initialize single linked lists and heaps
        result.clear();
        fHeap<long,long> heap; //one makes heap to have max first
        heap.sign = 1; //make heap decreasing

        std::vector<std::forward_list<long>> matchings(graph_size - source_count);
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
            if (matching_count >= 0) {
                heap.enqueue(i, matching_count);
            }
            matchings[i - source_count] = linked_nodes;
        }

        /*
         * Here we embed another brute force:
         * Check each level of set coverage, and start trying each of them.
         * This means: have a queue of all facilities that must be tried
         *
         */

        bool if_result = false;
        long previous_covered = 0;
//        gamma = 1;
        while (!if_result) {
            //should be ere in case treecover is not called. we should have zero antirank anyway
            std::fill(customer_antirank.begin(), customer_antirank.end(), 0);

            logger->start2("brute-force copying");
            std::vector<std::forward_list<long>> new_matchings = matchings;
            fHeap<long,long> new_heap = heap;
            logger->finish2("brute-force copying");

            std::vector<long> covered(this->source_count, 0);
            std::vector<long> result;
            long total_covered = 0;
            if_result = proceedLevel(covered, total_covered, new_heap, new_matchings, result); //recursion

            //check whether to raise gamma or raise demand
            double gamma_improvement = (double)(total_covered - previous_covered)/(double)source_count;
            logger->add2("total covered greedy", total_covered);
            if (previous_covered != 0) logger->add2("gamma improvement", gamma_improvement);
            logger->add2("gamma", this->gamma);
            if (if_result || total_covered < 0.9 * source_count || gamma_improvement == 0) {
                if (previous_covered != 0) gamma--;
                break;
            }
            previous_covered = total_covered;
            gamma++;
        }

        //return findSetCover result, with antirank updated as a global variable
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
            //calculate capacity increase vector based on covered and antirank
            //summarize covered and antirank vector. if covered is not full then antirank is zero. otherwise covered is one anywhere
            //so they do not interfere
            std::vector<long> speed(source_count);
            long max = 0;
            long total_uncovered = 0;
            for (long i = 0; i < source_count; i++) {
                speed[i] = this->customer_antirank[i];
                total_uncovered += (speed[i] == 0);
                max = std::max(speed[i], max);
            }
            if (max == 0) {
                for (long i = 0; i < speed.size(); i++) {
                    speed[i] = 1;
                }
            } else {
                double coef = this->alpha * (double) total_uncovered / (double)source_count;
                for (long i = 0; i < speed.size(); i++) {
                    speed[i] = (long)(coef * (double)speed[i]/(double)max);
                }
            }

            for (long vid = 0; vid < source_count; vid++) {
                for (long j = 0; j < speed[vid]; j++) {
                    this->increaseCapacity(vid);
                }
//                this->increaseCapacity(vid);
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
