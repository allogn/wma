//
// Created by allogn on 24.01.17.
//

#ifndef FCLA_SIA_H
#define FCLA_SIA_H

#include <string>
#include "igraph/igraph.h"
#include "nheap.h"

namespace SIA {

    typedef long large_int_t;
    long INF = LONG_MAX;
    typedef struct {
        std::string message = "There is no path in a bipartite graph from a source node to a non-full target node";
    } NO_PATH_ERROR;
    typedef fHeap<large_int_t> mmHeap;

    /*
     * Get Cost of an Edge according to potential values of vertices
     *
     * input: vtype is an indicator which subset a vertex belongs to
     */
    inline large_int_t edgeCost(igraph_bool_t source_vtype,
                                large_int_t weight,
                                large_int_t source_potential,
                                large_int_t target_potential)
    {
        if (source_vtype) weight = -weight; //invert weight of an edge if edge has an opposite direction
        return weight - source_potential + target_potential;
    };

    /*
     * Get enheaped cost of an edge
     */
    inline large_int_t heapedCost(large_int_t edge_w,
                                    large_int_t source_potential, //potential of source node of an edge
                                    large_int_t source_mindist) // minimum distance to a source node
    {
        return source_mindist + edge_w - source_potential;
    }

    /*
     * Update vector with minimum distance from a source node (Dijkstra execution)
     */
    inline bool updateMindist(igraph_vector_long_t* mindist,
                              igraph_integer_t v_id,
                              large_int_t new_distance)
    {
        large_int_t cur_dist = igraph_vector_long_e(mindist, v_id);
        if (cur_dist > new_distance) {
            igraph_vector_long_set(mindist, v_id, new_distance);
            return true;
        }
        return false;
    }

    /*
     * Check if a node has reached its capacity
     */
    inline bool isFull(igraph_t* bigraph, igraph_integer_t vid, large_int_t capacity) {
        igraph_vector_t res;
        igraph_vector_init(&res,1);
        igraph_degree(bigraph, &res, igraph_vss_1(vid), IGRAPH_OUT, false);
        return capacity - VECTOR(res)[0] > 0;
    }

    /*
     * Pop top edge from gheap and add it to the residual bigraph
     * Return ID of the new edge
     */
    //@todo move node insertion outside
//    igraph_integer_t insertEdgeFromHeap(mmHeap* gheap, igraph_t* bigraph)
//    {
//        igraph_integer_t source_id;
//        gheap->dequeue(source_id);
//
//        long eid = g->insert_nn_to_edge_list(fromid);
//        flow[eid] = 1; //one because an edge from noA and noB has different meanings on flow, it is residual flow
//
//        g->V[g->E[eid].fromid].E.push_back(eid);
//        heap_checkAndUpdateEdgeMin(globalH,g->E[eid].fromid); //MUST BE HERE
//        return eid;
//    }

//    /*
//     * After a new edge comes to a bipartite graph we must update Dijkstra Heap
//     * Before main Dijkstra execution we must update Dijkstra Heap according to
//     */
//    void updateHeaps(long eid, long fromid, long toid)
//    {
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
//    }

//    /*
//     * Update minimum distance in the global heap (heap that stores edges from all vertices)
//     *
//     * At each iteration of the algorithm (for each customer) at each moment of time (after each Dijkstra execution)
//     * global heap must store up-to-date information about visited vertices. Here we check if information should be
//     * updated and update if so.
//     */
//
//

//
//    long long totalflow;
//    long taumax;
//
//    int* flow; //residual flow in an edge
//    long* excess;
//    long long* psi;
//
//    long long* mindist;
//    long* mineid;
//    bool* watched;
//
//    long totalCost;
//

    /*
     * Continue running Dijkstra based on existing minimum distances and heap
     * Return the closest non-full service or -1 if there is no path
     */
    igraph_integer_t dijkstra(igraph_t* bigraph,
                              mmHeap* dheap,
                              igraph_vector_long_t* mindist,
                              igraph_vector_long_t* capacities,
                              igraph_vector_bool_t* types,
                              igraph_vector_long_t* potentials,
                              igraph_vector_long_t* weights,
                              igraph_vector_int_t* backtrack //store ids of an ancestor in a dijkstra tree
    )
    {
        long current_node; //must be of type "long" instead "igraph_integer_t". Otherwise change mmHeap type (idx is always of long type)
        while(dheap->dequeue(current_node) != 0) //heap is not empty
        {
            //if current node is of second type and is not full - return it
            //the degree of outgoing edges is how many customers the service is matched with
            if (VECTOR(*types)[current_node] && !isFull(bigraph,current_node,VECTOR(*capacities)[current_node])) {
                return current_node;
            }

            //update minimum distances to all neighbors
            igraph_vector_t eids;
            igraph_vector_init(&eids,0);
            igraph_incident(bigraph, &eids, current_node, IGRAPH_OUT);
            for (igraph_integer_t i = 0; i < igraph_vector_size(&eids); i++) {
                igraph_integer_t eid = VECTOR(eids)[i];
                igraph_integer_t target_node, source_node;
                igraph_edge(bigraph,eid,&source_node,&target_node);
                large_int_t new_cost = edgeCost(VECTOR(*types)[source_node],
                                                  VECTOR(*weights)[eid],
                                                  VECTOR(*potentials)[source_node],
                                                  VECTOR(*potentials)[target_node]);
                if (updateMindist(mindist, target_node, new_cost)) {
                    //update tree
                    igraph_vector_int_set(backtrack, target_node, source_node);
                    //update or enqueue target node
                    dheap->updateorenqueue(target_node, new_cost);
                }
            }
        }
        return -1; //no path exist
    }

    /*
     * Add a new edge to a residual bipartite graph
     */
//    bool addNewEdges() {
//        //adding new edge to the subgraph
//        long eid = insertEdgeFromHeap();
////            cout << "added " << g->E[eid].fromid << "->" << g->E[eid].toid << endl;
//
//        //updating Dijkstra heap by using additional heap
//        updateHeaps(eid, g->E[eid].fromid, g->E[eid].toid);
//        long curid;
//        long long tmp;
//        while (updateH.size() > 0) {
//            updateH.dequeue(curid, tmp);
//            for (long i = 0; i < g->V[curid].E.size(); i++) {
//                eid = g->V[curid].E[i];
//                updateHeaps(eid, curid, g->get_pair(eid, curid));
//            }
//        }
//        } while (dijkH.size() == 0);// || (globalH.size() > 0 && dijkH.getTopValue()>globalH.getTopValue()));
//    }

    /*
     * Initialize vectors and variables for Dijkstra
     */
    void init_dijsktra(large_int_t vcount,
                       mmHeap* dheap,
                       igraph_vector_long_t* mindist,
                       igraph_vector_long_t* potentials,
                       igraph_vector_long_t* backtrack,
                       igraph_integer_t source_id)
    {
        //initialize heaps and vectors according to a source node
        igraph_vector_long_init(mindist, vcount);
        igraph_vector_long_fill(mindist, INF);
        VECTOR(*mindist)[source_id] = 0;

        igraph_vector_long_init(potentials, vcount);
        igraph_vector_long_null(potentials); //init potentials with zero

        igraph_vector_long_init(backtrack, vcount);
        igraph_vector_long_fill(backtrack, -1); //init backtrack with -1 and itself for source
        igraph_vector_long_set(backtrack, source_id, source_id);

        igraph_vector_long_init(potentials, vcount);

        //enqueue first node into Dijktra heap
        dheap.enqueue(source_id, 0);
    }

    /*
     * Find a matching for a vertex in a bipartite graph
     *
     * In fact there is one continuous Dijkstra execution that is iteratively terminated
     * or started depending on current results and a stream of new edges
     */
    void match_vertex(igraph_t* bipartite, //residual bipartite graph
                      igraph_vector_long_t capacities,
                      igraph_integer_t source_id,
                      large_int_t (*get_nn_weight)(igraph_integer_t), //callback that returns weight of nearest vertex
                      igraph_integer_t (*get_nn)(igraph_integer_t), //callback that returns id of nearest vertex
                      void (*pop_nn)(igraph_integer_t) //callback that pops nearest neighbor
    )
    {
        large_int_t vcount = igraph_vcount(bipartite);

        //init heaps
        mmHeap gheap; //global heap stores information of the next not-added neighbor of vertices
        mmHeap dheap; //heap used in Dijkstra

        igraph_vector_long_t mindist; //minimum distance in Dijstra execution
        igraph_vector_long_t potentials;
        igraph_vector_long_t backtrack; //ids of ancestor in Dijkstra tree
        init_dijsktra(vcount, &dheap, &mindist, &potentials, &backtrack, source_id);

//        igraph_vector_bool_t visited; //visited nodes in Dijkstra execution

        large_int_t source_nn_weight = get_nn(source_id);
        //bipartite may not be empty, so there may not be additional outgoing edges for a feasible problem
        if (source_nn_weight != -1) gheap.enqueue(source_id, heapedCost(source_nn_weight, 0, 0));


        //@todo what is visited/watched for??

//        long target_node = -1;
//        watched[source_id] = 1;

        //while we have not found a non-full service - run Dijkstra and check for a threshold
        igraph_integer_t target_id = -1;
        while (target_id == -1) {
            //run Dijkstra
            target_id = dijkstra(bigraph,dheap,mindist,capacities,types,potentials,backtrack);
            //@todo update gheap for each visited node
            //            heap_checkAndUpdateEdgeMin(globalH,current_node);
            //            watched[current_node] = true;

            //if current residual graph is full, then return target_id or exception
            //@todo check that there can not be any neightbors of type B nodes
            if (target_id == -1) {
                if (!addNewEdge()) {
                    throw NO_PATH_ERROR;
                }
            } else {
                large_int_t sp_length = VECTOR(mindist)[target_id];
                if (sp_length < gheap.getTopValue() && addNewEdge()) {
                    target_id = -1; //invalidate result if new edge is added, otherwise return current result
                }
            }
        }
        augmentFlow(target_node);
        raisePotentials();

        //raise potentials and clear everything after iteration
//        iteration_reset(target_node);
    }


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
