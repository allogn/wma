//
// Created by allogn on 24.01.17.
//

#ifndef FCLA_SIA_H
#define FCLA_SIA_H

#include <string>
#include <igraph/igraph.h>
#include "nheap.h"

namespace SIA {

    long INF = LONG_MAX;
    typedef struct {
        std::string message = "There is no path in a bipartite graph from a source node to a non-full target node";
    } NO_PATH_ERROR;
    typedef fHeap<long> mmHeap;
    typedef struct {
        bool exists; //flag if there is any new edge to be added
        igraph_integer_t source_node;
        igraph_integer_t target_node;
        long weight;
        long capacity;
    } newEdge;
    typedef std::vector<newEdge> newEdges;

    /*
     * Get Cost of an Edge according to potential values of vertices
     *
     * input: vtype is an indicator which subset a vertex belongs to
     */
    inline long edgeCost(long weight,
                         long source_potential,
                         long target_potential)
    {
        //do not invert weight of an edge, because inverted edges are explicitly added to a graph
        return weight - source_potential + target_potential;
    };

    /*
     * Get enheaped cost of an edge
     */
    inline long heapedCost(long edge_w,
                           long source_potential, //potential of source node of an edge
                           long source_mindist) // minimum distance to a source node
    {
        return source_mindist + edge_w - source_potential;
    }

    /*
     * Update vector with minimum distance from a source node (Dijkstra execution)
     */
    inline bool updateMindist(igraph_vector_long_t* mindist,
                              igraph_integer_t v_id,
                              long new_distance)
    {
        long cur_dist = igraph_vector_long_e(mindist, v_id);
        if (cur_dist > new_distance) {
            igraph_vector_long_set(mindist, v_id, new_distance);
            return true;
        }
        return false;
    }

//
//    /*
//     * After a new edge comes to a bipartite graph we must update Dijkstra Heap
//     * Before main Dijkstra execution we must update Dijkstra Heap according to a new coming edge
//     *
//     * We don't want to run a new dijkstra execution, but instead update current dijkstra heap
//     * by considering all vertices until ones in the frontier of the main dijkstra
//     */
//    void updateHeaps(mmHeap* dheap, mmHeap* gheap, igraph_integer_t start_vid)
//    {
//
//        if (watched[fromid] == 0) return; //case when prefinal node : not all neighbours are considered => not watched,
//        // but in globalH and in DijkH (!). so, if in dijkH => everything is fine (will be updated later)
//        if (updateMinDist(eid,fromid,toid)) {
//            //no isUpdated because enheap only if updated dist
//            if (!dijkH.isExisted(toid)) {
//                //isUpdated omitted here
//                if (watched[toid] == 1) heap_checkAndUpdateMin(updateH, toid, mindist[toid]);
//                else dijkH.enqueue(toid, mindist[toid]);
//            } else {
//                dijkH.updatequeue(toid, mindist[toid]);
//            }
//        }
//
//
//        long curid;
//        long long tmp;
//        while (updateH.size() > 0) {
//            updateH.dequeue(curid, tmp);
//            for (long i = 0; i < g->V[curid].E.size(); i++) {
//                eid = g->V[curid].E[i];
//                updateHeaps(eid, curid, g->get_pair(eid, curid));
//            }
//        }
//
//    }


    /*
     * Initialize vectors and variables for Dijkstra
     */
    void init_dijsktra(long vcount,
                       mmHeap* dheap,
                       igraph_vector_long_t* mindist,
                       igraph_vector_long_t* potentials,
                       igraph_vector_t* backtrack,
                       igraph_integer_t source_id)
    {
        //initialize heaps and vectors according to a source node
        igraph_vector_long_init(mindist, vcount);
        igraph_vector_long_fill(mindist, INF);
        VECTOR(*mindist)[source_id] = 0;

        igraph_vector_long_init(potentials, vcount);
        igraph_vector_long_null(potentials); //init potentials with zero

        igraph_vector_init(backtrack, vcount);
        igraph_vector_fill(backtrack, -1); //init backtrack with -1 and itself for source
        igraph_vector_set(backtrack, source_id, source_id);

        igraph_vector_long_init(potentials, vcount);

        //enqueue first node into Dijktra heap
        dheap->enqueue(source_id, 0);
    }

    /*
     * Continue running Dijkstra based on existing minimum distances and heap
     * Return the closest non-full service or -1 if there is no path
     */
    igraph_integer_t dijkstra(igraph_t* bigraph,
                              mmHeap* dheap,
                              mmHeap* gheap,
                              newEdges& nearest_edges,
                              igraph_vector_long_t* mindist, //distance from a source node
                              igraph_vector_long_t* edge_excess, //amount of "free" flow = capacity - actual flow
                              igraph_vector_bool_t* types,
                              igraph_vector_long_t* potentials,
                              igraph_vector_long_t* weights,
                              igraph_vector_t* backtrack //store ids of an ancestor in a dijkstra tree
    )
    {
        long current_node; //must be of type "long" instead "igraph_integer_t". Otherwise change mmHeap type (idx is always of long type)
        while(dheap->dequeue(current_node) != 0) //heap is not empty
        {
            //if current node is of second type and is not full - return it
            //the degree of outgoing edges is how many customers the service is matched with
            if (VECTOR(*types)[current_node]) {
                //enheap back the last node in order to return the same (best) result
                dheap->enqueue(current_node, VECTOR(*mindist)[current_node]);
                return current_node; //already contains correct path distance in both backtrack and mindist arrays
            }
            //update minimum distances to all neighbors
            igraph_vector_t eids;
            igraph_vector_init(&eids,0);
            igraph_incident(bigraph, &eids, current_node, IGRAPH_OUT);
            for (igraph_integer_t i = 0; i < igraph_vector_size(&eids); i++) {
                igraph_integer_t eid = VECTOR(eids)[i];
                //check if the current edge is full then do not consider it
                if (VECTOR(*edge_excess)[eid] == 0) continue;
                //otherwise update distance to target node
                igraph_integer_t target_node, source_node;
                igraph_edge(bigraph,eid,&source_node,&target_node);
                long new_cost = VECTOR(*mindist)[source_node];
                new_cost += edgeCost(VECTOR(*weights)[eid],
                                     VECTOR(*potentials)[source_node],
                                     VECTOR(*potentials)[target_node]);
                if (updateMindist(mindist, target_node, new_cost)) {
                    //update breadcrumbs
                    igraph_vector_set(backtrack, target_node, source_node);
                    //update or enqueue target node
                    dheap->updateorenqueue(target_node, new_cost);
                    //maintain global heap (value for target_node was changed because of mindist
                    long nearest_weight = nearest_edges[target_node].weight;
                    long new_edge_cost = heapedCost(nearest_weight, VECTOR(*potentials)[target_node], new_cost);
                    if (nearest_weight != -1) gheap->updateorenqueue(target_node, new_edge_cost);
                }
            }
            igraph_vector_destroy(&eids);
        }
        return -1; //no path exist
    }

    /*
     * Add a new edge to a residual bipartite graph
     */
    void addNewEdge(igraph_t* bigraph,
                    mmHeap* dheap,
                    //newEdge (*next_neighbor)(long target_node), @todo move outside
                    newEdge new_edge,
                    igraph_vector_long_t* weights,
                    igraph_vector_long_t* excess,
                    igraph_vector_long_t* potentials,
                    igraph_vector_long_t* mindist)
    {
        //get the best edge from a global heap or return if the heap is full
//        long source_node;
//        if (!gheap->dequeue(source_node))
//            return false; //@todo move this outside

        //add a new edge
        igraph_add_edge(bigraph, new_edge.source_node, new_edge.target_node);
        igraph_vector_long_push_back(weights, new_edge.weight);
        igraph_vector_long_push_back(excess, new_edge.capacity);
        //add an inverted edge to a bigraph
        igraph_add_edge(bigraph, new_edge.target_node, new_edge.source_node);
        igraph_vector_long_push_back(weights, -new_edge.weight);
        igraph_vector_long_push_back(excess, 0);
        //@todo check if input graph has bidirectional nodes (bipartite graph) - if so - delete

        //updating Dijkstra heap by adding source_node to a heap:
        //note, that we don't have to check all outgoing edges, but consideration of the new edge
        //is nothing but a part of dijkstra execution starting from source node,
        //so we do a bit of overhead in order to minimize amount of code
        if (VECTOR(*mindist)[new_edge.source_node] < INF) //otherwise we will have overflow of long when calculating INF+something as new distance
            dheap->updateorenqueue(new_edge.source_node, VECTOR(*mindist)[new_edge.source_node]);

        // update vector with next nearest weights
//        newEdge next_new_edge = next_neighbor(new_edge.source_node);
//        nearest_edges[new_edge.source_node] = next_new_edge;
        // enqueue the next new value in gheap
//        if (new_edge.exists) {
//            long new_heaped_value = heapedCost(next_new_edge.weight,
//                                               VECTOR(*potentials)[source_node],
//                                               VECTOR(*mindist)[source_node]);
//            gheap->enqueue(source_node, new_heaped_value);
//        }
    }


    /*
     * Find a matching for a vertex in a bipartite graph
     *
     * In fact there is one continuous Dijkstra execution that is iteratively terminated
     * or started depending on current results and a stream of new edges
     */
//    void match_vertex(igraph_t* bigraph, //residual bipartite graph
//                      igraph_vector_bool_t* types,
//                      igraph_vector_long_t* capacities,
//                      igraph_integer_t source_id,
//            // nearest_weights : weights of the next nearest neighbors (init once), used for gheap updates
//            // must be initialized with the first nearest neighbor for each node
//                      igraph_vector_long_t* nearest_weights,
//                      newEdges& nearest_edges, // global cache for storing vertex ids that correspond to nearest_weights
//                      newEdge (*next_neighbor)(igraph_integer_t) //callback that returns weight of next nearest vertex
//    )
//    {
//        large_int_t vcount = igraph_vcount(bigraph);
//
//        //init heaps
//        mmHeap gheap; //global heap stores information of the next not-added neighbor of vertices
//        mmHeap dheap; //heap used in Dijkstra
//
//        igraph_vector_long_t mindist; //minimum distance in Dijstra execution
//        igraph_vector_long_t potentials;
//        igraph_vector_long_t backtrack; //ids of ancestor in Dijkstra tree
//        init_dijsktra(vcount, &dheap, &mindist, &potentials, &backtrack, source_id);
//
////        igraph_vector_bool_t visited; //visited nodes in Dijkstra execution
//
//        long source_nn_weight = VECTOR(*nearest_weights)[source_id];
//        //bipartite may not be empty, so there may not be additional outgoing edges for a feasible problem
//        if (source_nn_weight != -1) gheap.enqueue(source_id, heapedCost(source_nn_weight, 0, 0));
//
//
//        //@todo what is visited/watched for??
//
//        //while we have not found a non-full service - run Dijkstra and check for a threshold
//        igraph_integer_t target_id = -1;
//        while (target_id == -1) {
//            //run Dijkstra
//            target_id = dijkstra(bigraph,dheap,gheap,nearest_weights,mindist,capacities,types,potentials,backtrack);
//            //            heap_checkAndUpdateEdgeMin(globalH,current_node);
//            //            watched[current_node] = true;
//
//            //if current residual graph is full, then return target_id or exception
//            if (target_id == -1) {
//                if (!addNewEdge(bigraph, &gheap, &dheap, next_neighbor, nearest_edges, &potentials, &mindist)) {
//                    throw NO_PATH_ERROR;
//                }
//            } else {
//                large_int_t sp_length = VECTOR(mindist)[target_id];
//                if (sp_length < gheap.getTopValue() &&
//                        addNewEdge(bigraph, &gheap, &dheap, next_neighbor, nearest_edges, &potentials, &mindist)) {
//                    target_id = -1; //invalidate result if new edge is added, otherwise return current result
//                } //otherwise return current target_id as valid
//            }
//        }
//        augmentFlow(target_node);
//        raisePotentials();
//
//        //raise potentials and clear everything after iteration
////        iteration_reset(target_node);
//
//        //free memory
//        igraph_vector_long_destroy(&mindist);
//        //@todo others
//    }


//
//    int run(igraph_t* bigraph, igraph_real_t* result_weight, igraph_vector_long_t* result_matching) {
//        //init variables
//        fHeap<long long> dijkH; //heap for Dijkstra
//        fHeap<long long> globalH; //heap for edges
//        fHeap<long long> updateH; //heap for managing updates
//
//        totalflow = 0;
//        taumax = 0;
//
//        excess = new long[noA+noB];
//        fill(excess,excess+noA,1);
//        fill(excess+noA,excess+noA+noB,-1);
//
//        flow = new int[g->m];
//        fill(flow,flow+g->m,1);
//
//        psi = new long long[noA+noB];
//        fill(psi,psi+noA+noB,0);
//
//        mindist = new long long[noA+noB];
//        fill(mindist,mindist+noA+noB,LONG_MAX);
//
//        mineid = new long[noA+noB];
//        fill(mineid,mineid+noA+noB,-1);
//
//        watched = new bool[noA+noB];
//        fill(watched,watched+noA+noB,0);
//
//        //run main loop: round-robin until all vertices are matched
//        long q = 0;
//        while (totalflow < noA) {
//            processId(q);
//            q++;
//            if (q >= noA) q = 0;
//        }
//
//        //@todo calculate total cost here and return to &result_weight
//    }


//    void SIA::augmentFlow(long lastid) {
//        long curid = lastid;
//        long eid, toid;
//        while(mineid[curid] != -1)
//        {
//            //reverse edges
//            eid = mineid[curid];
//            toid = curid;
//            curid = g->get_pair(eid,curid);
//
//            int cur_flow = (curid < noA)?flow[eid]-1:flow[eid]+1;
//            assert(cur_flow == 0 || cur_flow == 1);
//            flow[eid] = cur_flow;
//
//            //deleting edges with no residual cost and moving them to the list of a reverse edge
//            if ((cur_flow == 0 && curid < noA) || (cur_flow == 1 && curid >= noA))
//            {
//                //delete edge from curid node edge list
//                long i = 0;
//                while (g->get_pair(g->V[curid].E[i++],curid) != toid);
//                g->V[curid].E[--i] = *g->V[curid].E.rbegin();
//                g->V[curid].E.pop_back();
//            }
//
//            //adding edge to a reverse edge if necessary
//            long i = 0;
//            while (i < g->V[toid].E.size() && g->get_pair(g->V[toid].E[i++],toid) != curid);
//            if (i == g->V[toid].E.size()) {
//                g->V[toid].E.push_back(eid);
//            }
//        }
//    }
//

//
//    void SIA::iteration_reset(long nodeid_best_psi) {
//        long best_psi = mindist[nodeid_best_psi];
//        dijkH.clear();
//        updateH.clear();
//        globalH.clear();
//        for (long nodeid = 0; nodeid < g->n; nodeid++) {
//            if (mindist[nodeid] < best_psi) {
//                psi[nodeid] = psi[nodeid] - mindist[nodeid] + best_psi;
//            }
//        }
//        fill(mindist,mindist+noA+noB,std::numeric_limits<long long>::max());
//        fill(mineid,mineid+noA+noB,-1);
//        fill(watched,watched+noA+noB,0);
//    }

}

#endif //FCLA_SIA_H
