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
    newEdges edgeMemory;

    void save(std::string filename) {
        ofstream f;
        f.open(filename,std::ofstream::out);
        for (long i = 0; i < edgeMemory.size(); i++) {
            f << edgeMemory[i].exists << ","
              << edgeMemory[i].target_node << ","
              << edgeMemory[i].capacity << ","
              << edgeMemory[i].source_node << ","
              << edgeMemory[i].weight << endl;
        }
        f.close();
    }

    EdgeGenerator() {};
    virtual ~EdgeGenerator() {};
    virtual newEdge getEdge(igraph_integer_t vid) {
        newEdge e;
        e.exists = false;
        return e;
    };
};

/*
 * Loads file with edgeMemory (edge queue) and throws all edges sequentially
 */
class LoadedEdgeGenerator : public EdgeGenerator {
public:
    newEdges edgeQueue;
    LoadedEdgeGenerator(std::string filename) {
        this->load(filename);
    }
    ~LoadedEdgeGenerator() {}
    void load(std::string filename) {
        ifstream f(filename);
        if (!f.is_open())
            throw "File does not exist";
        std::string line;
        while ( getline (f,line) ) {
            if (line == "")
                throw "Empty file with Edges";
            std::vector<long> vect;
            std::stringstream ss(line);
            long i;
            while (ss >> i)
            {
                vect.push_back(i);

                if (ss.peek() == ',')
                    ss.ignore();
            }
            newEdge e;
            e.exists = vect[0];
            e.target_node = vect[1];
            e.capacity = vect[2];
            e.source_node = vect[3];
            e.weight = vect[4];
            edgeMemory.push_back(e);
        }
        f.close();
        edgeQueue = edgeMemory;
    }

    newEdge getEdge(igraph_integer_t vid) override {
        for (long i = 0; i < this->edgeQueue.size(); i++) {
            if (edgeQueue[i].source_node == vid) {
                newEdge e = edgeQueue[i];
                edgeQueue.erase(edgeQueue.begin() + i);
                return e;
            }
        }
        newEdge new_edge;
        new_edge.exists = false;
        return new_edge;
    }
};

/*
 * Class that provides a callback function that incrementally throws new edges for each node in weight increasing order
 */
class RandomEdgeGenerator : public EdgeGenerator {
public:
    long n;
    std::vector<long> prev_weights;
    std::vector<std::vector<long>> neighbors;
    std::vector<long> next_neighbor_id;
    igraph_rng_t rng;
    long capacity;

    /*
     * Create a generator for n nodes
     *
     * Specify a range for target nodes and capacity of target nodes
     * Every edge will have a capacity equal to capacity of a target node
     */
    RandomEdgeGenerator(long n, long target_start_vid, long target_vcount, long target_capacity) {
        this->capacity = target_capacity;
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
            new_edge.capacity = this->capacity;
            edgeMemory.push_back(new_edge);
            prev_weights[vid] = new_edge.weight;
        } else {
            new_edge.exists = false;
        }
        return new_edge;
    }
};

#endif //FCLA_EDGEGENERATOR_H
