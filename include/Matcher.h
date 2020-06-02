/*
 * Match in a bipartite graph. Target and Source node ids correspond to ids in the bipartite graph.
 * Edge Generator used to get weights for new edges, returning source and target id corresponding to a bipartite graph.
 *
 * Supports undercapacitated facilities: adds one extra node with required excess inflow. All nodes matched to that node
 * are considered "unmatched".
 */

#ifndef FCLA_MATCHER_H
#define FCLA_MATCHER_H

#include <string>
#include <vector>
#include <forward_list>
#include <igraph/igraph.h>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "nheap.h"
#include "helpers.h"
#include "EdgeGenerator.h"
#include "TargetExploringEdgeGenerator.h"
#include "Logger.h"
#include "exceptions.h"
#include "Hilbert.h"

/*
 * Template types stand for
 * < flow/supply type, weight/potentials/cost type, node/edge index type >
 */
template<typename F, typename W, typename I>
class Matcher {
public:
    const W INF_W = std::numeric_limits<W>::max();
    const W VERY_BIG_W = 1000000; // weight for uncapacitated-case extra node
    //bigraph as two linked lists, storing only non-full edges
    typedef std::pair<I,W> Edge;
    typedef std::forward_list<Edge> Adjlist;
    typedef typename Adjlist::iterator EdgeIterator;
    std::vector<Adjlist> edges; // decreasing order of weights
    std::vector<std::vector<Edge>> backwards_edges; //for greedy, increasing order
    std::vector<F> node_excess;
    std::vector<F> full_node_excess;
    std::vector<F> total_matched; //already matched facilities to a customer
    EdgeGenerator* edge_generator;
    Network* network;
    Logger* logger;
    bool allow_extra_node_assignment;
    bool greedyMatching = false;
    bool hilbert_is_ready = false;
    std::vector<long> hilbert_order;
    int greedyMatchingOrder = 0;

    //handle uncapacitated case
    std::vector<bool> extra_edge_added_per_source;

    //global variables (throughout the algorithm)
    std::vector<W> potentials;
    I graph_size;
    newEdges new_edges;
    I source_count; //target count is graph_size-source_count, maybe refactor later

    //local variables (preserve only for one iteration)
    std::vector<W> mindist;
    std::vector<I> backtrack;
    fHeap<W,I> dheap;
    fHeap<W,I> gheap; //@todo what about enheaping the first node? what about dist of all nodes of dheap between iterations?

    //arrays with results
    W result_weight;

    void empty_new_edges() {
        new_edges.clear();
        for (I i = 0; i < graph_size; i++){
            newEdge e;
            e.exists = false;
            new_edges.push_back(e);
        }
    }

    I inline getExtraNodeIndex() {
        return this->node_excess.size()-1;
    }

    void reset() {
        total_matched.clear();
        total_matched.resize(source_count, 0);

        potentials.resize(graph_size, 0);
        edges.resize(graph_size);
        backwards_edges.resize(graph_size);
        for (I i = 0; i < graph_size; i++) {
            edges[i].clear();
            backwards_edges[i].clear();
        }

        extra_edge_added_per_source.clear();
        extra_edge_added_per_source.resize(source_count, false);

        //init with a first nearest neighbor for all source vertices
        edge_generator->reset();
        new_edges.clear();
        for (I i = 0; i < source_count; i++){
            newEdge e = edge_generator->getEdge(i);
            if (!e.exists && !this->extra_edge_added_per_source[i]) {
                this->extra_edge_added_per_source[i] = true;
                e.exists = true;
                e.weight = VERY_BIG_W;
                e.source_node = i;
                e.capacity = 1;
                e.target_node = getExtraNodeIndex();
            }
            new_edges.push_back(e);
        }

        mindist.resize(graph_size);
        backtrack.resize(graph_size);
    }

    //for Facility Location inheritance
    Matcher() {}

    Matcher(EdgeGenerator* edge_generator, std::vector<F>& node_excess, Logger* logger, bool allow_extra_node_assignment=true) {
        this->edge_generator = edge_generator;
        this->graph_size = edge_generator->n + edge_generator->m + 1;
        this->source_count = edge_generator->n;
        this->node_excess = node_excess;
        this->allow_extra_node_assignment = allow_extra_node_assignment;

        // extra node must have enough excess to serve all nodes, consider multicomponent graph
        this->node_excess.push_back(std::numeric_limits<long>::max()); // big number goes from the fact that later we can increase demads of customers

        this->logger = logger;
        reset();
    }

    ~Matcher() {}

    /*
     * Get Cost of an Edge according to potential values of vertices
     *
     * input: vtype is an indicator which subset a vertex belongs to
     */
    inline W edgeCost(W weight, I source_id, I target_id)
    {
        //do not invert weight of an edge, because inverted edges are explicitly added to a graph
        return weight - potentials[source_id] + potentials[target_id];
    };

    /*
     * Get enheaped cost of an edge
     */
    inline W heapedCost(W new_edge_w, I source_id)
    {
        return mindist[source_id] + new_edge_w - potentials[source_id];
    }

    inline bool ifValidTarget(W distance) {
        return distance < this->gheap.getTopValue();
    }

    /*
     * Update vector with minimum distance from a source node (Dijkstra execution)
     */
    inline bool updateMindist(I v_id, W new_distance)
    {
        W cur_dist = mindist[v_id];
        if (cur_dist > new_distance) {
            mindist[v_id] = new_distance;
            return true;
        }
        return false;
    }

    /*
     * Initialize vectors and variables for Dijkstra
     */
    void iteration_init(I source_id)
    {
        //init heaps
        gheap.clear(); //global heap stores information of the next not-added neighbor of vertices
        dheap.clear(); //heap used in Dijkstra

        //initialize heaps and vectors according to a source node
        std::fill(mindist.begin(),mindist.end(), INF_W);
        mindist[source_id] = 0;

        std::fill(backtrack.begin(), backtrack.end(), -1);
        backtrack[source_id] = source_id;

        //enqueue first node into Dijktra heap
        dheap.enqueue(source_id, 0);
    }

    /*
     * Continue running Dijkstra based on existing minimum distances and heap
     * Return the closest non-full service or -1 if there is no path
     */
    I dijkstra()
    {
        I current_node; //must be of type "long" instead "igraph_integer_t". Otherwise change mmHeap type (idx is always of long type)
        while(dheap.dequeue(current_node) != 0) //heap is not empty
        {
            //if current node is of second type and is not full - return it
            //the degree of outgoing edges is how many customers the service is matched with
            if (node_excess[current_node] > 0) {
                //enheap back the last node in order to return the same (best) result
                dheap.enqueue(current_node, mindist[current_node]);
                return current_node; //already contains correct path distance in both backtrack and mindist arrays
            }
            //update minimum distances to all neighbors
            for (EdgeIterator it = edges[current_node].begin(); it != edges[current_node].end(); it++) {
                W new_cost = mindist[current_node];
                I target_node = it->first;
                new_cost += edgeCost(it->second, current_node, target_node);
                if (updateMindist(target_node, new_cost)) {
                    //update breadcrumbs
                    backtrack[target_node] = current_node;
                    //update or enqueue target node
                    dheap.updateorenqueue(target_node, new_cost);
                    //maintain global heap (value for target_node was changed because of mindist
                    //note that target_node may not be among source nodes, but new_edges array has a size of source nodes only
                    if (target_node < this->source_count) {
                        newEdge nearest_edge = new_edges[target_node];
                        W new_edge_cost = heapedCost(nearest_edge.weight, target_node);
                        if (nearest_edge.exists) gheap.updateorenqueue(target_node, new_edge_cost);
                    }
                }
            }
        }
        return -1; //no path exist
    }

    /*
     * Add a new edge to a residual bipartite graph and update dijkstra heap accordingly
     */
    void addNewEdge(newEdge new_edge)
    {
        //add a new edge
        edges[new_edge.source_node].push_front(std::make_pair(new_edge.target_node, new_edge.weight));


        //updating Dijkstra heap by adding source_node to a heap:
        //note, that we don't have to check all outgoing edges, but consideration of the new edge
        //is nothing but a part of dijkstra execution starting from source node,
        //so we do a bit of overhead in order to minimize amount of code
        if (mindist[new_edge.source_node] < INF_W) //otherwise we will have overflow of long when calculating INF+something as new distance
            dheap.updateorenqueue(new_edge.source_node, mindist[new_edge.source_node]);
    }

    inline newEdge getEdgeToExtraNode(long source_node) {
        newEdge e;
        e.exists = true;
        e.weight = VERY_BIG_W;
        e.capacity = 1;
        e.source_node = source_node;
        e.target_node = getExtraNodeIndex();
        return e;
    }

    /*
     * Try to add edge from a global heap, update gheaps if succeed
     *
     * Return if something was added
     */
    bool addHeapedEdge()
    {
        //get the best edge from a global heap or return if the heap is empty
        I source_node;
        if (!gheap.dequeue(source_node))
            return false;
        addNewEdge(new_edges[source_node]);

        // update vector with next nearest weights
        newEdge next_new_edge = edge_generator->getEdge(source_node);
        new_edges[source_node] = next_new_edge;
        // enqueue the next new value in gheap
        if (next_new_edge.exists) {
            W new_heaped_value = heapedCost(next_new_edge.weight, source_node);
            gheap.enqueue(source_node, new_heaped_value);
        } else {
            //if no more edges to enheap - add an edge to extra node for that source node
            //do it only once per node
            if (!this->extra_edge_added_per_source[source_node]) {
                this->extra_edge_added_per_source[source_node] = true;
                W new_heaped_value = VERY_BIG_W;
                gheap.enqueue(source_node, new_heaped_value);
                new_edges[source_node] = getEdgeToExtraNode(source_node);
            }
            //else ignore
        }
        return true;
    }

    /*
     * Run dijkstra and enlarge graph until a valid path to non-full vertex to type B appears
     *
     * Since there is an extra node, the source node will be matched to at least one as a result
     *
     * Return false if no path exists
     */
    long runHeapDijkstraAndEnlargeBGraph()
    {
        long target_id = -1;
        while (target_id == -1) {
            target_id = dijkstra();
            if (target_id == -1) {
                if (!addHeapedEdge()) {
                    throw no_more_edges_to_add_exception;
                }
            } else {
                W sp_length = mindist[target_id];
                //check threshold
                if (!ifValidTarget(sp_length) && addHeapedEdge()) {
                    target_id = -1; //invalidate result if new edge is added, otherwise return current result
                }
            }
        }
        return target_id;
    }

    /*
     * Update potentials for all visited nodes
     *
     * Maintain potentials so that there are no negative weights: new = old + (dist[target] - dist[current])
     * Do it for all nodes where dist < dist[target] (mindist), finding nodes by BFS
     */
    void updatePotentials(I target)
    {
        W target_distance = mindist[target];
        for (I i = 0; i < graph_size; i++) {
            if (mindist[i] < target_distance) {
                potentials[i] = potentials[i] + target_distance - mindist[i];
            }
        }
    }

    /*
     * Change a flow (excess) in the graph from a source to a target (flip edges)
     * A path is reconstructed based on backtrack array, a source node is one that points to itself
     * - Inverted edges already must exist in the graph
     * - Ignore heap values as they will not be used anymore
     *
     */
    F augmentFlow(I target)
    {
        /*
         * Any augmentation is always 1 because each edge can handle max 1 flow (==assignment), even if
         * there is begger capacities between source and target
         * This is specific to the application, because multicapacity of a customer
         * means that this customer explores more, but not that there are many customers at one place
         */
        I current_node = target;
        //find minimum excess on the way and make it maximum flow increase
        F min_excess = INF_W;

        //iterate throw forward path and reassign edges to the opposite nodes (flip them)
        while (backtrack[current_node] != current_node) {
            //find an edge in adj list
            I target_node = current_node; //note this! we are back-propagating
            I source_node = backtrack[current_node];

            //find an edge
            EdgeIterator it = edges[source_node].begin();
            //we should erase *it, change weight to the opposite and assign to target
            //note that linked list has only "remove after" operation
            W weight;
            if (it->first == target_node) {
                weight = -it->second;
                edges[source_node].pop_front();
            } else {
                EdgeIterator prev_it = it;
                it++;
                while(true) { //strong assumption on graph structure consistency
                    if (it->first == target_node) {
                        break;
                    }
                    prev_it = it;
                    it++;
                }
                weight = -it->second;
                edges[source_node].erase_after(prev_it);
            }
            edges[target_node].push_front(std::make_pair(source_node,weight));
            current_node = source_node;
        }

        //current node contains the source node after while loop, so we change node_excess
        node_excess[target] -= 1;
        total_matched[current_node] += 1;
        node_excess[current_node] += 1;

        return 1;
    }


    /*
     * Find a matching for a vertex in a bipartite graph
     *
     * In fact there is one continuous Dijkstra execution that is iteratively terminated
     * or started depending on current results and a stream of new edges
     *
     * Returns flow change
     */
    F matchVertex(I source_id)
    {
        if (node_excess[source_id] >= 0)
            throw std::logic_error("Only nodes with negative excess can be matched");

        this->iteration_init(source_id);

        //nearest_edges array is global, but gheap is local. In order to descrease heap size we enheap
        //only those nodes which were visited by the algorithm (relevant)
        //if a node was not yet visited, then we should enheap the value from nearest_edges
        //this is done in dijkstra, because gheap is updated by the current value from nearest_edges if mindist is updated
        if (new_edges[source_id].exists) //otherwise all edges were already added, but everything is still fine
            gheap.enqueue(source_id, heapedCost(new_edges[source_id].weight, source_id));

        //enlarge graph until valid path is found, or throw an exception
        long result_vid = runHeapDijkstraAndEnlargeBGraph();

        F flowChange = augmentFlow(result_vid);
        updatePotentials(result_vid);

        return flowChange;
    }

    /*
     * Run matching algorithm on a bipartite graph with vcountA,vcountB number of vertices of two types
     *
     * No not pass bipartite &graph, but a function that provides incremental edges
     */
    void match() {
        if (this->greedyMatching) {
            this->matchGreedy();
            return;
        }

        I source_id = 0;
        I prev_id = -1;
        //round-robin
        long flowChangePerCycle = 0;
        while (source_id != prev_id) {
            while (node_excess[source_id] < 0) { //match source_id while there is any negative excess
                matchVertex(source_id);
//                print_edgelist();
//                if (matchVertex(source_id) == 0) {
//                    throw "Unfeasible problem: not enough facilities/customers";
//                }
                prev_id = source_id;
            }
            source_id = (source_id+1) % source_count;
        }
    }

    std::vector<long> matchGreedy() {
        // take some order of customers, assign to the nearest available facility, return result
        logger->start("greedyMatching");
        std::vector<long> source_ids(this->edge_generator->n);
        for (long i = 0; i < source_ids.size(); i++) {
            source_ids[i] = i;
        }
        std::vector<long> explored_sources;
        switch (this->greedyMatchingOrder) {
            case 2: this->makeHilbertSourceOrder(source_ids); break;
            case 3: this->makeDistanceSourceOrder(source_ids); break;
            default:
                this->makeRandomSourceOrder(source_ids);
                break;
        }
        for (auto it = source_ids.begin(); it != source_ids.end(); it++) {
            if (!this->matchToClosestAvailableFacility(*it)) {
                explored_sources.push_back((*it));
            }
        }
        logger->finish("greedyMatching");
        return explored_sources;
    }

    void makeRandomSourceOrder(std::vector<long>& sources) {
        std::random_shuffle(sources.begin(), sources.end());
    }

    struct Customer {
        Coords coords;
        long index;
    };

    struct ComparatorHilbertEntry
    {
        bool operator()(const Customer& r1, const Customer& r2)
        {
            double d1[2],d2[2];
            d1[0] = r1.coords.first;
            d1[1] = r1.coords.second;
            d2[0] = r2.coords.first;
            d2[1] = r2.coords.second;
            return hilbert_ieee_cmp(2,d1,d2) >= 0;
        }
    } hilbert_comparator;

    void makeHilbertSourceOrder(std::vector<long>& sources) {
        if (this->hilbert_is_ready) {
            sources = this->hilbert_order;
            return;
        }
        std::vector<Customer> customers;
        for (auto s : sources) {
            Coords coords = this->getCustomerCoords(s);
            customers.push_back({coords, s});
        }
        std::cout << "begin hilbert" << std::endl;
        std::sort(customers.begin(), customers.end(), hilbert_comparator);
        for (long i = 0; i < sources.size(); i++) {
            sources[i] = customers[i].index;
        }
        this->hilbert_order = sources;
        this->hilbert_is_ready = true;
        std::cout << "hilbert ready" << std::endl;
    }

    Coords getCustomerCoords(long source_id) {
        return Coords(this->network->coords[network->source_indexes[source_id]].first, this->network->coords[network->source_indexes[source_id]].second);
    }

    void makeDistanceSourceOrder(std::vector<long>& sources) {
        //distance to the nearest available facility

        //zip with distances
        std::vector<std::pair<long, long>> zipped_sources;
        for (auto s : sources) {
            zipped_sources.push_back(std::make_pair(this->getDistanceToClosestAvailableFacility(s), s));
        }

        std::sort(zipped_sources.begin(), zipped_sources.end());
        for (long i = 0; i < sources.size(); i++) {
            sources[i] = zipped_sources[i].second;
        }
    }

    long getDistanceToClosestAvailableFacility(long source_id) {
        for (auto edge : this->backwards_edges[source_id]) {
            if (!this->ifTargetCapacitated(edge.first)) {
                return edge.second;
            }
        }
        return VERY_BIG_W;
    }

    bool matchToClosestAvailableFacility(long source_id) {
        if (this->node_excess[source_id] == 0) {
            return true;
        }
        auto it = backwards_edges[source_id].begin();
        W weight;
        long target_node;
        long closestFacility = 0;
        while (this->node_excess[source_id] < 0) {
            while (it == backwards_edges[source_id].end() || this->ifTargetCapacitated(it->first)) {
                closestFacility++;
                if (it == backwards_edges[source_id].end()) {
                    newEdge new_edge = this->edge_generator->getEdge(source_id);
                    if (!new_edge.exists) {
                        logger->add(std::string("furthest traversal ") + std::to_string(source_id), -1);
                        return false;
                    }
                    edges[source_id].push_front(std::make_pair(new_edge.target_node, new_edge.weight));
                    backwards_edges[source_id].push_back(std::make_pair(new_edge.target_node, new_edge.weight));
                    it = std::prev(backwards_edges[source_id].end());
                } else {
                    it++;
                }
            }
            weight = -it->second;
            target_node = it->first;
            edges[target_node].push_front(std::make_pair(source_id,weight));
            node_excess[source_id]++;
            node_excess[target_node]--;

            it++; // because we can not match twice with the same facility
        }
        logger->add(std::string("furthest traversal ") + std::to_string(source_id), closestFacility);
        return true;
    }



    /*
     * Increase the demand of a particular customer
     */
    F increaseCapacity(I vid) {
        //if current vid is already has demand equal to every possible facility - ignore
        if (total_matched[vid] >= this->edge_generator->m) {
            return 0;
        }

        //only one value per call because only one matchvertexroutine is called, that can change flow value on max 1 value
        //in bipartite graph
        this->node_excess[vid] -= 1;

        if (this->greedyMatching) {
            this->full_node_excess[vid] -= 1;
            return 1;
        }

        //before capacity was changed, all excesses were zero. Then, one changed
        //As a result of matchVertex routine, no any other vertex can loose its matching
        //because any path is "continuous"
        int result;
        try {
            result = matchVertex(vid);
        } catch (NoMoreEdgesToAdd& e) {
            /*
             * this is allowed here, because there is only one extra node, but customers are allowed to be matched
             * with each facility only once,so some customers may need more extra nodes
             */
            result = 0;
        }
        if (result == 0) {
            this->node_excess[vid] += 1; //do not match this node ever
        }
        return result;
    }

    inline bool ifAllSourceMatchedExactlyOnce() {
        std::vector<bool> is_matched(this->edge_generator->n, false);
        for (long i = this->edge_generator->n; i < this->edge_generator->n + this->edge_generator->m; i++) {
            for (auto e : this->edges[i]) {
                if (is_matched[e.first]) {
                    return false;
                }
                is_matched[e.first] = true;
            }
        }
        this->logger->add("checked if all matched", "yes");
        //check if any of sources were not matched to any of facilities, hence matched to an extra node
//        for(auto const& outedge: edges[getExtraNodeIndex()]) {
//            return false;
//        }
        return true;
    }

    void calculateResult() {
        //check if there are some nodes that are matched to an excess node. if so - throw exception
        //however the matching is valid
        if (!allow_extra_node_assignment && !ifAllSourceMatchedExactlyOnce()) {
            throw std::logic_error("WMA produced infesible solution");
        }

        //arrange an answer
        result_weight = 0;
        for (I i = source_count; i < graph_size-1; i++) { //one goes for an extra node
            for (EdgeIterator it = edges[i].begin(); it != edges[i].end(); it++) {
                result_weight += abs(it->second);
            }
        }
    }

    bool ifTargetCapacitated(long target_id) {
        assert(target_id >= this->edge_generator->n && target_id < this->node_excess.size());
        return this->node_excess[target_id] == 0;
    }

    inline long get_bi_node_id_by_target_id(long target_id) {
        return target_id + this->source_count;
    }

    inline long get_bi_outdegree(long bi_node_id) {
        long i = 0;
        for (auto &x : this->edges[bi_node_id]) i++;
        return i;
    }

    void print_edgelist() {
        for (long i = 0; i < this->edges.size(); i++) {
            std::cout << i << ": ";
            for (auto it = this->edges[i].begin(); it != this->edges[i].end(); it++) {
                std::cout << "(" << it->first << "," << it->second << "), ";
            }
            std::cout << std::endl;
        }
    }

};

#endif //FCLA_MATCHER_H
