//
// Created by allogn on 24.01.17.
//

#ifndef FCLA_MATCHER_H
#define FCLA_MATCHER_H

#include <string>
#include <vector>
#include <igraph/igraph.h>
#include <limits>
#include <algorithm>
#include "nheap.h"
#include "helpers.h"
#include "EdgeGenerator.h"

/*
 * Template types stand for
 * < flow/supply type, weight/potentials/cost type, node/edge index type >
 */
template<typename F, typename W, typename I>
class Matcher {
public:
    const W INF_W = std::numeric_limits<W>::max();
    igraph_t graph;
    EdgeGenerator* edge_generator;

    //global variables (throughout the algorithm)
    std::vector<W> potentials;
    std::vector<F> node_excess;
    std::vector<F> edge_excess;
    std::vector<W> weights;
    I graph_size;
    I source_count;
    newEdges new_edges;

    //local variables (preserve only for one iteration)
    std::vector<W> mindist;
    std::vector<I> backtrack;
    fHeap<W,I> dheap;
    fHeap<W,I> gheap; //@todo what about enheaping the first node? what about dist of all nodes of dheap between iterations?

    //arrays with results
    W result_weight;
    std::vector<std::vector<I>> result_matching;

    void empty_new_edges() {
        new_edges.clear();
        for (I i = 0; i < graph_size; i++){
            newEdge e;
            e.exists = false;
            new_edges.push_back(e);
        }
    }

    void reset() {
        potentials.resize(graph_size, 0);
        weights.clear();
        edge_excess.clear();
        result_matching.clear();
        for (I i = 0; i < graph_size; i++) {
            std::vector<I> vec;
            result_matching.push_back(vec);
        }

        //init with a first nearest neighbor for all vertices
        edge_generator->reset();
        new_edges.clear();
        for (I i = 0; i < graph_size; i++){
            newEdge e = edge_generator->getEdge(i);
            new_edges.push_back(e);
        }

        source_count = -1;
        for (I i = 0; i < graph_size; i++) {
            if ((node_excess[i] >= 0) && (source_count == -1)) {
                source_count = i;
            }
            if ((node_excess[i] < 0) && (source_count != -1)) {
                throw "Node Excess array must contain first sources, then targets";
            }
        }
        mindist.resize(graph_size);
        backtrack.resize(graph_size);
    }

    //for testing purposes
    Matcher(I graph_size) {
        this->graph_size = graph_size;
        igraph_empty(&graph, graph_size, true);
        mindist.resize(graph_size);
        backtrack.resize(graph_size);
    }

    Matcher(igraph_t* g) {
        igraph_copy(g, &graph);
        this->graph_size = igraph_vcount(g);
        mindist.resize(graph_size);
        backtrack.resize(graph_size);
    }

    Matcher(EdgeGenerator* edge_generator, std::vector<W>& node_excess) {
        graph_size = node_excess.size();
        igraph_empty(&graph, graph_size, true); //create directed graph
        this->edge_generator = edge_generator;
        this->node_excess = node_excess;
        reset();
    }

    ~Matcher() {
        igraph_destroy(&graph);
    }

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
            igraph_vector_t eids;
            igraph_vector_init(&eids,0);
            igraph_incident(&graph, &eids, current_node, IGRAPH_OUT);
            for (igraph_integer_t i = 0; i < igraph_vector_size(&eids); i++) {
                igraph_integer_t eid = VECTOR(eids)[i];
                //check if the current edge is full then do not consider it
                if (edge_excess[eid] == 0) continue;
                //otherwise update distance to target node
                igraph_integer_t target_node, source_node;
                igraph_edge(&graph,eid,&source_node,&target_node);
                W new_cost = mindist[source_node];
                new_cost += edgeCost(weights[eid], source_node, target_node);
                if (updateMindist(target_node, new_cost)) {
                    //update breadcrumbs
                    backtrack[target_node] = source_node;
                    //update or enqueue target node
                    dheap.updateorenqueue(target_node, new_cost);
                    //maintain global heap (value for target_node was changed because of mindist
                    newEdge nearest_edge = new_edges[target_node];
                    W new_edge_cost = heapedCost(nearest_edge.weight, target_node);
                    if (nearest_edge.exists) gheap.updateorenqueue(target_node, new_edge_cost);
                }
            }
            igraph_vector_destroy(&eids);
        }
        return -1; //no path exist
    }

    /*
     * Add a new edge to a residual bipartite graph and update dijkstra heap accordingly
     */
    void addNewEdge(newEdge new_edge)
    {
        //add a new edge
        igraph_add_edge(&graph, new_edge.source_node, new_edge.target_node);
        weights.push_back(new_edge.weight);
        edge_excess.push_back(new_edge.capacity);
        //add an inverted edge to a bigraph
        igraph_add_edge(&graph, new_edge.target_node, new_edge.source_node);
        weights.push_back(-new_edge.weight);
        edge_excess.push_back(0);

        //updating Dijkstra heap by adding source_node to a heap:
        //note, that we don't have to check all outgoing edges, but consideration of the new edge
        //is nothing but a part of dijkstra execution starting from source node,
        //so we do a bit of overhead in order to minimize amount of code
        if (mindist[new_edge.source_node] < INF_W) //otherwise we will have overflow of long when calculating INF+something as new distance
            dheap.updateorenqueue(new_edge.source_node, mindist[new_edge.source_node]);
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
        }
        return true;
    }

    /*
     * Run dijkstra and enlarge graph until a valid path to non-full vertex to type B appears
     *
     * Return false if no path exists
     */
    bool findAndEnlarge(I* result_vid)
    {
        igraph_integer_t target_id = -1;
        while (target_id == -1) {
            //run Dijkstra
            target_id = dijkstra();

            //if current residual graph is full, then return target_id or exception
            if (target_id == -1) {
                if (!addHeapedEdge()) {
                    return false;
                }
            } else {
                W sp_length = mindist[target_id];
                //check threshold
                if (!ifValidTarget(sp_length) && addHeapedEdge()) {
                    target_id = -1; //invalidate result if new edge is added, otherwise return current result
                }
            }
        }
        *result_vid = target_id;
        return true;
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
        I current_node = target;
        //find minimum excess on the way and make it maximum flow increase
        F min_excess = INF_W;

        //path selectors, igraph needed for igraph_es_pairs call
        igraph_vector_t forward_vids;
        igraph_vector_t backward_vids;
        igraph_vector_init(&backward_vids, 0);

        while (backtrack[current_node] != current_node) {
            igraph_vector_push_back(&backward_vids, current_node);
            igraph_vector_push_back(&backward_vids, backtrack[current_node]);
            current_node = backtrack[current_node];
        }
        igraph_vector_copy(&forward_vids, &backward_vids);
        igraph_vector_reverse(&forward_vids);

        igraph_es_t forward_es;
        igraph_es_t backward_es;
        igraph_es_pairs(&forward_es, &forward_vids, true);//directed graph
        igraph_es_pairs(&backward_es, &backward_vids, true);

        //iterate through forward path and find maximum flow
        igraph_eit_t eit;
        igraph_eit_create(&graph, forward_es, &eit);
        while (!IGRAPH_EIT_END(eit)) {
            igraph_integer_t eid = IGRAPH_EIT_GET(eit);
            min_excess = min(min_excess, edge_excess[eid]);
            IGRAPH_EIT_NEXT(eit);
        }
        min_excess = min(min_excess, node_excess[target]);
        min_excess = min(min_excess, -node_excess[current_node]);

        //iterate throw both paths and change excess
        IGRAPH_EIT_RESET(eit);
        while (!IGRAPH_EIT_END(eit)) {
            igraph_integer_t eid = IGRAPH_EIT_GET(eit);
            edge_excess[eid] = edge_excess[eid] - min_excess;
            IGRAPH_EIT_NEXT(eit);
        }
        igraph_eit_destroy(&eit);
        igraph_eit_create(&graph, backward_es, &eit);
        while (!IGRAPH_EIT_END(eit)) {
            igraph_integer_t eid = IGRAPH_EIT_GET(eit);
            edge_excess[eid] = edge_excess[eid] + min_excess;
            IGRAPH_EIT_NEXT(eit);
        }
        igraph_eit_destroy(&eit);

        //current node contains the source node after while loop, so we change node_excess
        node_excess[target] -= min_excess;
        node_excess[current_node] += min_excess;

        igraph_vector_destroy(&backward_vids);
        igraph_vector_destroy(&forward_vids);
        return min_excess;
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
            throw "Only nodes with negative excess can be matched";

        this->iteration_init(source_id);

        //nearest_edges array is global, but gheap is local. In order to descrease heap size we enheap
        //only those nodes which were visited by the algorithm (relevant)
        //if a node was not yet visited, then we should enheap the value from nearest_edges
        //this is done in dijkstra, because gheap is updated by the current value from nearest_edges if mindist is updated
        if (new_edges[source_id].exists) //otherwise all edges were already added, but everything is still fine
            gheap.enqueue(source_id, heapedCost(new_edges[source_id].weight, source_id));

        //enlarge graph until valid path is found, or throw an exception
        I result_vid;
        if (!findAndEnlarge(&result_vid))
        {
            throw "No valid matching in the &graph, out of additional nodes"; //no path in complete graph
        }

        F flowChange = augmentFlow(result_vid);
        updatePotentials(result_vid);

//        print_graph<W,F>(&graph, weights, edge_excess);
        return flowChange;
    }

    /*
     * Run matching algorithm on a bipartite graph with vcountA,vcountB number of vertices of two types
     *
     * No not pass bipartite &graph, but a function that provides incremental edges
     */
    void run() {
        I source_id = 0;
        I prev_id = -1;
        //round-robin
        while (source_id != prev_id) {
            while (node_excess[source_id] < 0) {
                matchVertex(source_id);
                prev_id = source_id;
            }
            source_id = (source_id+1) % source_count;
        }
    }

    /*
     * Increase the demand of a particular customer
     */
    void increaseCapacity(I vid) {
        this->node_excess[vid] -= 1;
        //before capacity was changed, all excesses were zero. Then, one changed
        //As a result of matchVertex routine, no any other vertex can loose its matching
        //because any path is "continuous"
        matchVertex(vid);
    }

    void calculateResult() {
        //arrange an answer
        result_weight = 0;
        for (I i = 0; i < source_count; i++) {
            igraph_vector_t neis;
            igraph_vector_init(&neis,0);
            igraph_neighbors(&graph, &neis, i, IGRAPH_IN);
            igraph_integer_t matched_id;
            //there can be many matched vertices because of capacities
            for (I j = 0; j < igraph_vector_size(&neis); j++) {
                igraph_integer_t eid;
                igraph_get_eid(&graph, &eid, VECTOR(neis)[j], i, true, true);
                if (edge_excess[eid]>0) {
                    matched_id = VECTOR(neis)[j];
                    result_matching[i].push_back(matched_id);
                    result_matching[matched_id].push_back(i);
                    result_weight -= weights[eid]*edge_excess[eid];
                }
            }
            igraph_vector_destroy(&neis);
        }
    }

};

#endif //FCLA_MATCHER_H
