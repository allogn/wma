//
// Created by Alvis Logins on 02/02/2017.
//

#ifndef FCLA_EDGEGENERATOR_H
#define FCLA_EDGEGENERATOR_H

#include <igraph/igraph.h>
#include <lemon/list_graph.h>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>

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
    long n; //number of vertices for generation (left side)
    long m; //number of target vertices
    newEdges edgeMemory;

    void save(std::string filename) {
        std::ofstream f;
        f.open(filename,std::ofstream::out);
        for (long i = 0; i < edgeMemory.size(); i++) {
            f << edgeMemory[i].exists << ","
              << edgeMemory[i].target_node << ","
              << edgeMemory[i].capacity << ","
              << edgeMemory[i].source_node << ","
              << edgeMemory[i].weight << std::endl;
        }
        f.close();
    }

    void makeComplete() {
        for (long i = 0; i < this->n; i++) {
            while (!isComplete(i)) {
                getEdge(i); //add to memory and return new edge
            }
        }
    }

    void makeLemon(lemon::ListDigraph* g,
                   lemon::ListDigraph::ArcMap<long>* capacities,
                   lemon::ListDigraph::ArcMap<long>* weights) {
        this->makeComplete();
        std::vector<lemon::ListDigraph::Node> nodes;
        std::set<long> nodenum;
        //calculate nodes
        for (long i = 0; i < edgeMemory.size(); i++) {
            nodenum.insert(edgeMemory[i].source_node);
            nodenum.insert(edgeMemory[i].target_node);
        }
        for (long i = 0; i < nodenum.size(); i++) {
            lemon::ListDigraph::Node node = g->addNode();
            nodes.push_back(node);
        }
        for (long i = 0; i < edgeMemory.size(); i++) {
            lemon::ListDigraph::Arc e = g->addArc(nodes[edgeMemory[i].source_node], nodes[edgeMemory[i].target_node]);
            (*capacities)[e] = edgeMemory[i].capacity;
            (*weights)[e] = edgeMemory[i].weight;
        }
    }

    EdgeGenerator() {}
    virtual ~EdgeGenerator() {}
    virtual bool isComplete(long vid) {
        return true;
    }
    virtual newEdge getEdge(long vid) {
        newEdge e;
        e.exists = false;
        return e;
    }
    virtual void reset() {}
};

/*
 * Loads file with edgeMemory (edge queue) and throws all edges sequentially
 */
class LoadedEdgeGenerator : public EdgeGenerator {
public:
    newEdges edgeQueue;
    LoadedEdgeGenerator(std::string filename) {
        this->n = 0;
        this->m = 0;
        this->load(filename);
    }
    ~LoadedEdgeGenerator() {}
    void load(std::string filename) {
        edgeMemory.clear();
        std::ifstream f(filename);
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
            this->n = std::max(n, (long)e.source_node);
            this->m = std::max(m, (long)e.target_node);
        }
        f.close();
        edgeQueue = edgeMemory;
    }

    newEdge getEdge(long vid) override {
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

    bool isComplete(long vid) override {
        return true; //a node for the loaded graph is always assumed to contain all outgoing edges
    }

    void reset() override {
        edgeQueue = edgeMemory;
    }
};

/*
 * Class that provides a callback function that incrementally throws new edges for each node in weight increasing order
 */
class RandomEdgeGenerator : public EdgeGenerator {
public:
    std::vector<long> prev_weights;
    std::vector<std::vector<long>> neighbors;
    std::vector<long> next_neighbor_id;
    igraph_rng_t rng;

    /*
     * Create a generator for n nodes
     *
     * Specify a range for target nodes and capacity of target nodes
     * Every edge will have a capacity equal to capacity of a target node
     */
    RandomEdgeGenerator(long n, long target_start_vid, long target_vcount, long target_capacity) {
        this->n = n;
        this->m = target_vcount;
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
    newEdge getEdge(long vid) override {
        newEdge new_edge;
        if (vid < n && !isComplete(vid)) {
            new_edge.source_node = vid;
            new_edge.weight = igraph_rng_get_integer(&rng, prev_weights[vid], prev_weights[vid]+100);
            new_edge.target_node = neighbors[vid][next_neighbor_id[vid]++];
            new_edge.exists = true;
            new_edge.capacity = 1;
            edgeMemory.push_back(new_edge);
            prev_weights[vid] = new_edge.weight;
        } else {
            new_edge.exists = false;
        }
        return new_edge;
    }

    bool isComplete(long vid) override {
        return next_neighbor_id[vid] == neighbors[vid].size();
    }

    void reset() override {
        edgeMemory.clear();
        prev_weights.resize(n,0);
        next_neighbor_id.resize(n, 0);
    }
};

#endif //FCLA_EDGEGENERATOR_H
