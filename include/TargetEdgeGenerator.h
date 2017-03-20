/*
 * The purpose of this generator is to simultaneously throw edges from already partially explored graph
 * and use ExploringEdgeGenerator to retrieve distances to particular targets that are not yet explored
 */

#ifndef FCLA_TARGETEDGEGENERATOR_H
#define FCLA_TARGETEDGEGENERATOR_H

#include <vector>
#include <queue>
#include <algorithm>
#include <igraph/igraph.h>
#include "Matcher.h"
#include "EdgeGenerator.h"

class TargetEdgeGenerator : public EdgeGenerator {
public:
    Matcher<long,long,long>* matcher;
    EdgeGenerator* edge_explorer;
    std::vector<long> target_indexes;

    //this contains flags whether all targets in bipartite graph are already thrown by this generator
    //if yes but not all targets are thrown then continue exploring in edge_explorer until all targets are reached
    std::vector<std::vector<newEdge>> edgeQueue; //this serves to save already explored edges in sorted order
    std::vector<long> targets_reached;
    std::vector<long> is_target;

    //because we must have weights
    TargetEdgeGenerator(Matcher<long,long,long>* matcher, std::vector<long>& target_indexes) {
        this->matcher = matcher;
        this->edge_explorer = matcher->edge_generator;
        this->target_indexes = target_indexes;

        //get some variables from edge_explorer
        this->n = edge_explorer->n;
        this->m = target_indexes.size();
        reset();
    }
    ~TargetEdgeGenerator() {}

    newEdge getEdge(long vid) override {
        newEdge new_edge;
        //no outgoing edges needed anymore
        if (isComplete(vid)) {
            new_edge.exists = false;
            edgeMemory.push_back(new_edge);
            return new_edge;
        }
        this->targets_reached[vid]++;
        //some remains from explored edges
        if (edgeQueue[vid].size() > 0) {
            new_edge = edgeQueue[vid].back();
            edgeQueue[vid].pop_back();
            edgeMemory.push_back(new_edge);
            return new_edge;
        }
        //continue exploring
        while (true) {
            new_edge = edge_explorer->getEdge(vid);
            if (!new_edge.exists) {
                edgeMemory.push_back(new_edge);
                return new_edge;
            }
            if (is_target[new_edge.target_node - this->n] > -1) {
                new_edge.target_node = this->edge_explorer->n + is_target[new_edge.target_node - this->n];
                edgeMemory.push_back(new_edge);
                return new_edge;
            }
        }
    }

    bool isComplete(long vid) override {
        return targets_reached[vid] == target_indexes.size();
    }

    void reset() override {
        edgeMemory.clear();
        /*
         * Note that here we don't have a network, but an abstract edge generator. we store obtained information
         * in bipartite graph in matcher. so, in general non-network case, the right side is a set of all potential locations
         * and only id of a potential location can be returned by a generator
         */
        is_target.resize(this->matcher->edge_generator->m,-1); //size of bipartite is size of sources plus targets
        for (long i = 0; i < target_indexes.size(); i++) {
            is_target[target_indexes[i]] = i;
        }

        targets_reached.clear();
        targets_reached.resize(this->n,0);

        //fill queue with explored edges
        std::vector<newEdge> empty_vec;
        edgeQueue.clear();
        edgeQueue.resize(this->n, empty_vec);

        //traverse all memorized edges and add those which are relevant to the current targets
        for (long i = 0; i < this->matcher->edge_generator->edgeMemory.size(); i++) {
            newEdge e = this->matcher->edge_generator->edgeMemory[i];
            if (is_target[e.target_node - this->n] > -1) {
                e.target_node = this->n + is_target[e.target_node - this->n];
                edgeQueue[e.source_node].push_back(e);
            }
            //sort values
            std::sort(edgeQueue[e.source_node].begin(), edgeQueue[e.source_node].end(), [](const newEdge a, const newEdge b) {
                return a.weight > b.weight;
            });
        }
    }
};

#endif //FCLA_TARGETEDGEGENERATOR_H
