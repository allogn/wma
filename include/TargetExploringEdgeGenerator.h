/*
 * This does the same as exploring generator, but yields only those edges which go to preallocated facilities
 */

#ifndef FCLA_TARGETEXPLORINGEDGEGENERATOR_H
#define FCLA_TARGETEXPLORINGEDGEGENERATOR_H

#include "ExploringEdgeGenerator.h"

template<typename I, typename W>
class TargetExploringEdgeGenerator : public ExploringEdgeGenerator<I,W> {
public:
    std::vector<bool> is_target;
    std::vector<long> reverse_index;
    std::vector<newEdge> buffer;

    void reset() override {
        this->init_dijkstra();
        for (long i = 0; i < this->n; i++) {
            updateBuffer(i);
        }
    }

    TargetExploringEdgeGenerator(Network& network,
                                 std::vector<long>& target_indexes) : ExploringEdgeGenerator<I,W>(network) {
        this->m = target_indexes.size();
        this->buffer.resize(this->n);
        is_target.resize(igraph_vcount(&network.graph),false);
        reverse_index.resize(igraph_vcount(&network.graph),-1);
        for (long i = 0; i < target_indexes.size(); i++) {
            is_target[target_indexes[i]] = true;
            reverse_index[target_indexes[i]] = i;
        }

        this->reset();
    }

    long get_facility_id_by_node_id(long node_id) {
        return this->reverse_index[node_id];
    }

    void updateBuffer(long vid) {
        newEdge e;
        e.exists = false;
        if (vid < this->n) {
            while (this->dheaps[vid].size() > 0) {
                I next_vid;
                W shortest_dist;
                this->dheaps[vid].dequeue(next_vid, shortest_dist);
                this->visited[vid][next_vid] = true;
                this->updateNeighbors(vid, next_vid, shortest_dist);
                if (is_target[next_vid]) {
                    e.exists = true;
                    e.capacity = 1;
                    e.source_node = vid;
                    //for target node we must return ID of a facility
//                    assert(reverse_index[next_vid] >= 0);
                    e.target_node = getIndexOfFacilityInBGraph(next_vid);
                    e.weight = shortest_dist; //shortest distance between

//                    for (long i = 0; i < this->edgeMemory.size(); i++) {
//                        if ((this->edgeMemory[i].source_node == e.source_node) &&
//                                (this->edgeMemory[i].source_node == e.target_node)) {
//                            std::cout << "55--234234" << std::endl;
//                            exit(1);
//                        }
//                    }

                    this->edgeMemory.push_back(e);
                    break;
                }
            }
        }
        buffer[vid] = e;
    }

    inline long getIndexOfFacilityInBGraph(long facility_index_in_graph) {
        return this->n + reverse_index[facility_index_in_graph];
    }

    bool isComplete(long vid) override {
        return !buffer[vid].exists;
    }

    //get next neighbor of a customer with ID = vid. Corresponding node in the graph = source_node_index[vid]
    newEdge getEdge(long vid) override {
        newEdge e = buffer[vid];
        updateBuffer(vid);
        return e;
    }
};

#endif //FCLA_TARGETEXPLORINGEDGEGENERATOR_H
