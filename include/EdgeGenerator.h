//
// Created by Alvis Logins on 02/02/2017.
//

#ifndef FCLA_EDGEGENERATOR_H
#define FCLA_EDGEGENERATOR_H

#include <igraph/igraph.h>

typedef struct {
    bool exists; //flag if there is any new edge to be added
    igraph_integer_t source_node;
    igraph_integer_t target_node;
    long weight;
    long capacity;
} newEdge;
typedef std::vector<newEdge> newEdges;

class EdgeGenerator {
public:
    EdgeGenerator() {};
    virtual ~EdgeGenerator() {};
    virtual newEdge getEdge(igraph_integer_t vid) {
        newEdge e;
        e.exists = false;
        return e;
    };
};

/*
 * Class that provides a callback function that incrementally throws new edges for each node in weight increasing order
 */
class RandomEdgeGenerator : public EdgeGenerator {
public:
    long n;
    newEdges edgeMemory;
    std::vector<long> prev_weights;
    std::vector<std::vector<long>> neighbors;
    std::vector<long> next_neighbor_id;
    igraph_rng_t rng;

    /*
     * Create a generator for n nodes
     *
     * Specify a range for target nodes
     */
    RandomEdgeGenerator(long n, long target_start_vid, long target_vcount) {
        this->n = n;
        prev_weights.resize(n,0);

        igraph_rng_init(&rng, &igraph_rngtype_mt19937);
        igraph_rng_seed(&rng, time(NULL));

        for (long i = 0; i < n; i++) {
            std::vector<long> neighbors;
            for (long j = target_start_vid; j < target_start_vid + target_vcount; j++) {
                if (i != j)
                    neighbors.push_back(j);
            }
            std::random_shuffle(neighbors.begin(), neighbors.end());
            this->neighbors.push_back(neighbors);
        }

        next_neighbor_id.resize(n, 0);
    }

    ~RandomEdgeGenerator() {
        igraph_rng_destroy(&rng);
    }

    /*
     * Randomly choose next neighbor and weight
     * new weight is no smaller than previous for the same node
     * new neighbor must not be any of previous
     *
     * return non-existing edge for n larger than the one used in initialization
     */
    newEdge getEdge(igraph_integer_t vid) override {
        newEdge new_edge;
        if (vid < n && next_neighbor_id[vid] < neighbors[vid].size()) {
            new_edge.source_node = vid;
            new_edge.weight = igraph_rng_get_integer(&rng, prev_weights[vid], prev_weights[vid]+100);
            new_edge.target_node = neighbors[vid][next_neighbor_id[vid]++];
            new_edge.exists = true;
            new_edge.capacity = 1;
            edgeMemory.push_back(new_edge);
        } else {
            new_edge.exists = false;
        }
        return new_edge;
    }
};

#endif //FCLA_EDGEGENERATOR_H
