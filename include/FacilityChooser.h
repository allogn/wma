/*
 * Potential bugs/improvements:
 *
 * graph_size is defined as n+m in fcla, but n+m+1 in matcher (extra node), that is inconsistent.
 *
 *
 *
 */


#ifndef FCLA_FACILITYCHOOSER_H
#define FCLA_FACILITYCHOOSER_H

#include <forward_list>
#include <fstream>
#include <stack>
#include "nheap.h"
#include "ExploringEdgeGenerator.h"
#include "TargetExploringEdgeGenerator.h"
#include "Matcher.h"
#include "Network.h"
#include "Logger.h"
#include "helpers.h"
#include "FacilityRank.h"
#include "exceptions.h"

class FacilityChooser : public Matcher<long,long,long> {
public:
    enum State {
        UNINITIALIZED, NOT_LOCATED, LOCATED, INFEASIBLE, ERROR
    };
    long max_coverage;
    std::vector<long> result;
    long totalCost;
    long required_facilities;
    std::string exp_id;
    long facility_capacity;
    std::vector<long> source_indexes;
    std::map<long, long> source_reverse_index;
    std::vector<long> target_indexes;
    std::vector<long> target_capacities;
    State state = UNINITIALIZED;
    std::vector<long> customer_antirank; //number of facilities a customer is
    std::vector<long> last_used;
    double alpha;
    long capacity_iteration;//iteration ID for WMA, utilized in last_used for potential facilities
    long total_covered;

    bool uniform_capacities;
    bool partially_uniform; //used to calculate objective for uniform, but result for non-uniform
    bool all_nodes_available;

    int objective_matching = 1; //if objective is calculated as SIA

    /*
     * lambda is a parameter that states when to terminate the heap exploration
     * it is equal to a minimal service filling required for considering that service
     */
    long lambda;

    FacilityChooser() {}; //for testing

    /*
     * Check feasibility by number of components
     */
    void check_feasibility() {
        igraph_t* g = &(network->graph);
        igraph_integer_t components;
        igraph_vector_t membership;
        igraph_vector_init(&membership,0);
        igraph_clusters(g, &membership, 0, &components, IGRAPH_WEAK);
        logger->add("number of components", components);
        std::vector<long> customers_sum(components,0);
        for (long i = 0; i < this->source_count; i++) {
            customers_sum[VECTOR(membership)[this->source_indexes[i]]]++;
        }
        //todo nonequal capacities
        long total_facilities = 0;
        for (long i = 0; i < components; i++) {
            total_facilities += ceil((double) customers_sum[i] / (double) this->facility_capacity);
        }

        igraph_vector_destroy(&membership);
        if (total_facilities > this->required_facilities) {
            throw infeasible_solution;
        }

        if ((this->uniform_capacities) && (this->required_facilities * this->facility_capacity < source_count)) {
            throw infeasible_solution;
        }
        //@todo add check of facility_indexes
    }

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
                    bool partially_uniform = false) {
        logger->start2("fcla initialization");
        this->network = &network;
        this->exp_id = network.id;
        this->source_indexes = network.source_indexes;
        this->source_count = network.source_indexes.size();
        this->target_capacities = network.target_capacities; //can be empty
        this->target_indexes = network.target_indexes;
        this->facility_capacity = facility_capacity;
        this->logger = logger;
        this->alpha = alpha;
        this->required_facilities = facilities_to_locate;
        this->lambda = lambda;
        this->state = NOT_LOCATED;
        this->partially_uniform = partially_uniform; //false default

        this->uniform_capacities = target_capacities.size() == 0;
        this->all_nodes_available = target_indexes.size() == 0;

        logger->add("id", network.id);
        logger->add("number of facilities", facilities_to_locate);
        logger->add("capacity of facilities", facility_capacity);
        logger->add("lambda", lambda);
	    logger->add("uniform capacities", this->uniform_capacities);

        //create generator anyway
        if (this->all_nodes_available) {
            this->edge_generator = new ExploringEdgeGenerator<long, long>(network);
        } else {
            this->edge_generator = new TargetExploringEdgeGenerator<long, long>(network, network.target_indexes);
        }
        graph_size = edge_generator->n + edge_generator->m + 1;
        this->last_used.resize(edge_generator->m, -1);
        this->customer_antirank.clear();
        this->customer_antirank.resize(source_count);
        this->build_source_reverse_index();

        logger->add("bipartite graph size", graph_size);

        this->node_excess = this->get_node_excess();
        this->full_node_excess = this->get_node_excess();

        //reset variables of the matching algorithm
        reset();
        logger->finish("fcla initialization");
    }

    ~FacilityChooser() {
        delete this->edge_generator;
    }

    std::vector<long> get_node_excess() {
        std::vector<long> node_excess(this->graph_size, -1);
        if (this->uniform_capacities || this->partially_uniform) {
            for (long i = this->network->source_indexes.size(); i < graph_size; i++) {
                node_excess[i] = facility_capacity;
            }
        } else {
            for (long i = 0; i < this->target_indexes.size(); i++) {
                node_excess[this->get_bi_node_id_by_target_id(i)] = this->target_capacities[i];
            }
            node_excess[node_excess.size()-1]=1000000;//excess node
        }
        return node_excess;
    }

    void build_source_reverse_index() {
        for (long i = 0; i < this->source_indexes.size(); i++) {
            this->source_reverse_index[this->source_indexes[i]] = i;
        }
    }


    bool if_relative_gain_threshold() {
        /*
         * condition to terminate set cover when a new target is selected (outdated)
         */
        return ((result.size() > this->required_facilities) && ((double)(source_count - this->total_covered)/(double)source_count > 0.1)) ||
               ((result.size() > this->required_facilities + lambda) && ((double)(source_count - this->total_covered)/(double)source_count <= 0.1));
    }

    bool if_covered_is_enough(long matching_count) {
        /*
         * another condition to terminate set cover when a new target is selected (outdated, depends on lambda)
         */
        return (matching_count * (this->required_facilities - this->result.size() + lambda) < source_count - this->total_covered);
    }


    /*
     * Returns false if no set cover exists within current lambda
     *
     * Important: we can use all facilities except the extra one
     */
    bool findSetCover() {
        logger->start("set cover check time");
        std::fill(customer_antirank.begin(), customer_antirank.end(), 0);
        //initialize single linked lists and heaps
        this->result.clear();
        fHeap<FacilityRank,long> heap;
        std::vector<std::forward_list<long>> matching;
        heap.sign = 1; //make heap decreasing order
        this->fillQueueOfMatchedNodesPerFacility(matching, heap);
        if (heap.size() == 0) {
            return false; //out of available facilities for some reason (probably border case)
        }

        std::vector<long> covered(this->source_count, 0);
	    this->max_coverage = heap.getTopValue().coverage;
        bool if_result = greedySetCover(matching, covered, heap);
        this->updateCustomerAntirank(covered);
        logger->add2("total covered final", total_covered);

        logger->finish("set cover check time");
        return if_result;
    }

    void updateCustomerAntirank(std::vector<long>& covered) {
        for (long i = 0; i < source_count; i++) {
            this->customer_antirank[i] += (covered[i] == 0);
        }
    }

    /*
     * Rank facilities according to matched nodes and the last iteration when it was used
     */
     void fillQueueOfMatchedNodesPerFacility(std::vector<std::forward_list<long>>& matching, fHeap<FacilityRank,long>& heap) {
        matching.resize(this->get_facility_count()-1);//without extra node
        for (long i = source_count; i < graph_size-1; i++) {
            forward_list<long> linked_nodes;
            long target_id = this->get_target_id_by_bi_node_id(i);
            //there can be many matched vertices because of capacities
            long matching_count = 0;
            EdgeIterator it;
            for (it = edges[i].begin(); it != edges[i].end(); it++) {
                linked_nodes.push_front(it->first);
                matching_count++;
            }
            //put in a heap
            if (matching_count >= 0) {
                heap.enqueue(i, FacilityRank(matching_count,this->last_used[target_id]));
            }
            matching[target_id] = linked_nodes;
        }
    };


    bool greedySetCover(std::vector<std::forward_list<long>>& matching, std::vector<long>& local_covered, fHeap<FacilityRank,long>& heap) {
        long heap_iterations = 0;
        total_covered = 0;
        while (pickAnotherFacility(matching, local_covered, heap)) {
            heap_iterations++;
        }
//        std::cout << total_covered << std::endl;
//        logger->add2("greedy deheap iterations", heap_iterations);
        return (total_covered == source_count);
    }

    bool pickAnotherFacility(std::vector<std::forward_list<long>>& matching, std::vector<long>& local_covered, fHeap<FacilityRank,long>& heap) {
        if (heap.size() == 0) {
            return false;
        }

        long bi_node_id;
        FacilityRank rank(-1,-1);
        heap.dequeue(bi_node_id, rank);
        long init_target_coverage = rank.coverage;
        long target_id = this->get_target_id_by_bi_node_id(bi_node_id);

        //test if matching is not empty. If so - return false immediately
        if (matching[target_id].begin() == matching[target_id].end()) {
            return false;
        }
        // check which matched vertex is still not covered and delete covered ones
        long new_covered_by_target_count = this->remove_covered_customers_from_queue(matching[target_id], local_covered);

        if (new_covered_by_target_count != init_target_coverage) {
            //if the size was changed - enheap back
            //not guaranteed to have a non-decreasing matching count
            rank.coverage = new_covered_by_target_count;
            heap.enqueue(bi_node_id, rank);
            return true;
        } else {
            // otherwise add to the result and update coverage
            result.push_back(target_id);

            double relative_gain = (double) new_covered_by_target_count / (double) (source_count - total_covered);
//            logger->add2("relative gain", relative_gain);
//            logger->add2("matching count", new_covered_by_target_count);
//            logger->add2("left count", source_count - total_covered);

            if (result.size() > this->required_facilities + lambda) {
                return false;
            }
            //update usage history here
            this->last_used[target_id] = this->capacity_iteration;

            for (auto it = matching[target_id].begin(); it != matching[target_id].end(); it++) {
                //every node in single-linked list must not be covered yet
                local_covered[(*it)]++;
                total_covered++;
            }
            return true;
        }
    }

    long remove_covered_customers_from_queue(std::forward_list<long>& queue, std::vector<long>& coverage) {
        // linked list can delete only the NEXT element, so we are checking each next element,
        // and then the first one separately
        long non_covered_count = 0;
        auto it = queue.begin();
        auto prev_it = queue.begin();
        it++;
        while (it != queue.end()) {
            long pair_id = (*it);
            if (coverage[pair_id]) {
                //delete from a linked-list
                queue.erase_after(prev_it);
                it = prev_it;
            } else {
                non_covered_count++;
                prev_it = it;
            }
            it++;
        }
        //process the first one
        long first_node = (*queue.begin());
        if (coverage[first_node]) {
            queue.pop_front();
        } else {
            non_covered_count++;
        }
        return non_covered_count;
    }

    /*
     * return: true if at least one customer is newly matched
     */
    //increase capacities of everyone
    //calculate capacity increase vector based on covered and antirank
    //summarize covered and antirank vector. if covered is not full then antirank is zero. otherwise covered is one anywhere
    //so they do not interfere
    bool increaseCapacities(std::vector<int>& complete_sources) {

        std::vector<long> speed(source_count);
        long max = 0;
        long total_covered = 0;
        long total_complete = 0;
        for (long i = 0; i < source_count; i++) {
            speed[i] = this->customer_antirank[i];
            total_covered += (speed[i] == 0);//total covered
            total_complete += (complete_sources[i]);
            speed[i] *= (1 - complete_sources[i]); //do not increase those with all component explored even if uncovered (hopping for another set cover attempt)
            max = std::max(speed[i], max);
        }
        if (total_complete == source_count) {
            throw infeasible_solution;
        }
        if (max == 0) {
            for (long i = 0; i < speed.size(); i++) {
                speed[i] = (1-complete_sources[i]);
            }
        }
//        else {
//            double coef = 1 + this->alpha * (double) total_covered / (double)source_count;
//            for (long i = 0; i < speed.size(); i++) {
//                speed[i] = (long)(coef * (double)speed[i]/(double)max);
//            }
//        }

        if (this->greedyMatching) {
            this->resetAssignmentForGreedyMatching();
        }

        bool anychanges = false;
        long total_increased = 0;
        for (long vid = 0; vid < source_count; vid++) {
            for (long j = 0; j < speed[vid]; j++) {
                total_increased++;
                int success = this->increaseCapacity(vid);
                if (!success) {
                    complete_sources[vid] = 1; //fully explored component
                } else {
                    anychanges = true;
                }
            }
        }

        if (this->greedyMatching) {
            std::vector<long> newly_explored_sources = this->matchGreedy();
            anychanges = (newly_explored_sources.size() < total_increased);
            for (auto it = newly_explored_sources.begin(); it != newly_explored_sources.end(); it++) {
                complete_sources[(*it)] = 1;
            }
        }

        return anychanges;
    }

    void resetAssignmentForGreedyMatching() {
        for (auto i = 0; i < this->edge_generator->n; i++) {
            this->node_excess[i] = this->full_node_excess[i];
        }
        for (auto i = this->edge_generator->n; i < this->edges.size(); i++) {
            this->edges[i] = Adjlist();
            this->node_excess[i] = this->full_node_excess[i];
        }
    }

    /*
     * Select worst customers: find worst matching and select closest not-matched facility
     *
     * - Traverse customers and select better options for each one.
     *    - if a customer does not have any better options - skip him, he already is covered by best option
     * - Select top-k using heapsort to satisfy facility requirement
     *    - if heapsort is larger than k, then we can shrink size of heap, as we never use more than k
     *      because each element in heap is a new facility after deheaping
     *    - if all selected better options is less than required left facilities, then select any available facilities
     *      because everyone already have the best option so far
     *    - if two customers request for the same facility, then we need to select more facilities, but it is
     *      not likely, so we just ignore again if some particular facility is already chosen
     *      hoping that it will not be capacitated. if it will, then it goes for a random facility chosen later
     *      for this purpose we maintain a full heap, without reducing size, as we can pull more than k
     *
     */
    void locateRest() {
        long facilities_left = required_facilities - this->result.size();
        logger->add("facilities left after termination", facilities_left);
        if (facilities_left <= 0) {
            return; //no problem
        }
        logger->start("locate rest time");

        //mark which facility is chosen, created here because result is built several times
        std::vector<bool> result_flag(this->edge_generator->m, false);
        for (long i = 0; i < this->result.size(); i++) {
            result_flag[this->result[i]] = true;
        }

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
        //now source_best should contain each customer some non-infinite number
        //mark -1 in source_best nodes which we explored the target
        fHeap<long,long> partialHeapsort;
        for (long i = 0; i < this->edge_generator->edgeMemory.size(); i++) { //refactor this by saving in mather history per each node
            newEdge e = this->edge_generator->edgeMemory[i];
            if (source_best[e.source_node] >= 0) {
                partialHeapsort.enqueue(e.target_node-this->edge_generator->n, source_best[i] - e.weight);
                source_best[e.source_node] = -1;
            }
        }

        //here we deheap ID of customer in source index; then, we should place there facility
        //result should contain id of facility in target_index array
        long new_location;
        while ((facilities_left > 0) && (partialHeapsort.size() > 0)) {
            partialHeapsort.dequeue(new_location);
            if (!result_flag[new_location]) {
                this->result.push_back(new_location); //put location in a network
                result_flag[new_location] = true;
                facilities_left--;
            }
        }

        if (facilities_left > 0) {
            for (long i = 0; i < result_flag.size(); i++) {
                if (!result_flag[i]) {
                    this->result.push_back(i);
                    facilities_left--;
                }
                if (facilities_left == 0) {
                    break;
                }
            }
        }

        logger->finish("locate rest time");
    }

    void locateFacilities() {
        logger->start("runtime");
        logger->start("matching");
        this->match(); //calculate preliminary matching
        logger->finish("matching");

        // increase customer capacities until we can choose a covering subset of matched services
        this->capacity_iteration = 0; //used for ranking @todo move to parameters
        std::vector<int> complete_sources(source_count, 0);
        while (!this->findSetCover()) {
            capacity_iteration++;
            logger->start("matching");
//            std::cout << this->total_covered << std::endl;
            if (!increaseCapacities(complete_sources)) {
                //try more - check if set cover result is the same. no more facilities only if absolutely all customers are full
                std::cout << "nothing added with coverage " << this->total_covered << std::endl;

                //remove one last facility to fix the size of the result
                this->result.pop_back();
                break;
                //throw no_more_capacities_to_increase;
            }
            logger->finish("matching");
        }
        logger->add("number of iterations", capacity_iteration);
        locateRest(); //locate rest of facilities (if a set that covers customers is smaller than required number of facilities)
        this->state = LOCATED;

        logger->finish("runtime");
    }

    inline long get_node_id_by_facility_id(long facility_id) {
        return (this->all_nodes_available) ? facility_id : this->target_indexes[facility_id];
    }

    inline long get_capacity_by_facility_id(long facility_id) {
        return (this->uniform_capacities || this->partially_uniform) ? this->facility_capacity : this->target_capacities[facility_id];
    }

    inline long get_facility_id_by_node_id(long node_id) {
        return (this->all_nodes_available) ? node_id : dynamic_cast<TargetExploringEdgeGenerator<long,long>*>(this->edge_generator)->get_facility_id_by_node_id(node_id);
    }

    inline long get_source_id_by_node_id(long node_id) {
        return this->source_reverse_index[node_id];
    }

    inline long get_bi_node_id_by_target_id(long target_id) {
        return target_id + this->source_count;
    }

    inline long get_target_id_by_bi_node_id(long node_id) {
        return node_id - this->source_count;
    }

    inline long get_bi_node_id_by_target_node_id(long node_id) {
        return this->get_bi_node_id_by_target_id(this->get_facility_id_by_node_id(node_id));
    }

    inline long get_facility_count() {
        return graph_size - source_count; //todo what about excess node?
    }

    std::vector<long> get_chosen_facility_node_ids() {
        std::vector<long> node_ids = this->result;
        for (auto it = node_ids.begin(); it != node_ids.end(); it++) {
            *it = this->get_node_id_by_facility_id(*it);
        }
        return node_ids;
    }

    long calculateResult() {
        //calculate Total Sum
        if (this->state != LOCATED) {
            throw std::logic_error("Facilities should be located before computing result");
        }
        logger->start("result final calculation time");

        //run matching in resulting biparite graph, but having capacity of 1 only and having customers
        //on the right side of bipartite graph

        //build valid bipartite graph : reverse all edges to the direct position
        //create an edge generator with calculated distances between known facilities and customers
        //run new matcher

        std::string facility_index_list = "";
        std::vector<long> new_excess(this->source_indexes.size() + this->result.size());
        for (long i = source_indexes.size(); i < new_excess.size(); i++) {
            long facility_id = this->result[i-source_indexes.size()];
            new_excess[i] = this->get_capacity_by_facility_id(facility_id);
            facility_index_list += std::to_string(this->get_node_id_by_facility_id(facility_id)) + ",";
        }
        logger->add("facilities_indexes", facility_index_list);

        for (long i = 0; i < this->source_indexes.size(); i++) {
            new_excess[i] = -1;
        }
        std::vector<long> chosen_node_ids = this->get_chosen_facility_node_ids();
        TargetExploringEdgeGenerator<long, long> bigraph_generator(*this->network, chosen_node_ids);
        Matcher<long,long,long> M(&bigraph_generator, new_excess, this->logger, false);
        M.greedyMatching = this->greedyMatching * this->objective_matching; //objective matching 0 means there should be SIA for objective calculation
        M.greedyMatchingOrder = this->greedyMatchingOrder;
        M.network = this->network;
        M.match();
        M.calculateResult(); // we CARE here if some customers are assigned to the extra node
        this->totalCost = M.result_weight;

        //calculate number of fully capacitated nodes
        long capn = 0;
        for (long i = this->source_indexes.size(); i < M.node_excess.size(); i++) {
            capn += (M.node_excess[i] == 0);
        }
        logger->add1("full facilities", capn);

        logger->finish("result final calculation time");
        logger->add("objective", totalCost);
        return this->totalCost;
    }

    void run() {
        this->check_feasibility();
        try {
            this->locateFacilities();
        } catch (NoMoreCapacitiesToIncrease& e) {
            throw infeasible_solution;
        }
        this->calculateResult();
    }
};

#endif //FCLA_FACILITYCHOOSER_H
