/*
 * Exploring Edge generator extands edge generator for bipartite graph:
 * For each customer it provides next nearest potential facility location
 */

#ifndef FCLA_EXPLORINGEDGEGENERATOR_H
#define FCLA_EXPLORINGEDGEGENERATOR_H

#include <vector>
#include <limits>
#include "EdgeGenerator.h"
#include "nheap.h"

template<typename I, typename W>
class ExploringEdgeGenerator : public EdgeGenerator {
public:
    const W INF_W = std::numeric_limits<W>::max();
    //provide dijkstra in various graph frameworks
    igraph_t* graph;// do not init or destroy
    I node_count_in_network; //note that there is <n> inherited for number of customers
    std::vector<fHeap<W,I>> dheaps; //dijsktra heaps for each source node
    std::vector<I> source_node_index; //index of customers: source_node_index[id] = vid in graph of a customer #id
    std::vector<W> weights;
    long facility_capacity = 1;

    /*
     * We run <this->n> heap-based dijsktras: if a node was deheaped, the distance is guaranteed to be minimal
     * for each neighbor : it can be in a heap (so should be updated), or it was deheaped, or it has INF distance
     * so mark each deheaped node as "visited", but for each dijkstra execution separately
     */
    std::vector<std::vector<bool>> visited;

    void updateNeighbor(I customer_id, I target, W cost) {
        fHeap<W,I>& dheap = dheaps[customer_id];
        if (dheap.isExisted(target)) {
            if (dheap.getVal(target) > cost) {
                dheap.updatequeue(target, cost);
            }
        } else {
            //check if visited
            if (visited[customer_id][target] == false) {
                dheap.enqueue(target, cost);
            } //else ignore neighbor
        }
    }

    //here we use igraph api
    void updateNeighbors(I customer_id, I vid, W cur_w) {
        igraph_vector_t neis;
        igraph_vector_init(&neis,0);
        igraph_neighbors(graph, &neis, vid, IGRAPH_ALL); //the graph is undirected (!) - now we work with a road map
        for (I i = 0; i < igraph_vector_size(&neis); i++) {
            I neig_vid = VECTOR(neis)[i];

            igraph_integer_t eid;
            igraph_get_eid(graph, &eid, vid, neig_vid, false, true);
            W new_dist = cur_w + weights[eid];

            updateNeighbor(customer_id, neig_vid, new_dist);
        }
        igraph_vector_destroy(&neis);
    }

    void init_dijkstra() {
        visited.clear();
        dheaps.clear();
        //n goes for number of customers
        for (I i = 0; i < n; i++) {
            fHeap<W,I> heap;
            dheaps.push_back(heap);

            std::vector<bool> v(node_count_in_network, false);
            visited.push_back(v);

            visited[i][source_node_index[i]] = true;
            updateNeighbors(i, source_node_index[i], 0);
        }
    }

    /*
     * edge generator generates edges for each customer, that has a particular place in a network g
     * source_node_index for each customer stores v_id of a node in g where the customer is located
     */
    ExploringEdgeGenerator(igraph_t* g,
                           std::vector<W>& weights,
                           std::vector<I>& source_node_index,
                           long facility_capacity)
    {
        //init dijkstra heaps
        node_count_in_network = igraph_vcount(g);
        this->n = source_node_index.size();
        this->source_node_index = source_node_index;
        this->graph = g;
        this->weights = weights;
        this->facility_capacity = facility_capacity;
        init_dijkstra();
    }
    ~ExploringEdgeGenerator() {}

    bool isComplete(long vid) override {
        return dheaps[vid].size() == 0;
    }

    //get next neighbor of a customer with ID = vid. Corresponding node in the graph = source_node_index[vid]
    newEdge getEdge(long vid) override {
        newEdge e;
        if ((vid >= this->n) || (isComplete(vid))) {
            e.exists = false;
        } else {
            e.exists = true;
            I next_vid;
            W shortest_dist;
            dheaps[vid].dequeue(next_vid, shortest_dist);
            visited[vid][next_vid] = true;
            updateNeighbors(vid, next_vid, shortest_dist);
            e.capacity = facility_capacity;
            e.source_node = vid;
            //for target node we must return ID of a facility, i.e.
            e.target_node = source_node_index.size() + next_vid;
            e.weight = shortest_dist; //shortest distance between
            edgeMemory.push_back(e);
        }
        return e;
    }

    //this should reset edge memory
    void reset() override {
        init_dijkstra();
    }
};

#endif //FCLA_EXPLORINGEDGEGENERATOR_H
