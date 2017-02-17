//
// Created by Alvis Logins on 17/02/2017.
//

#ifndef FCLA_NETWORK_H
#define FCLA_NETWORK_H

#include <string>
#include <igraph/igraph.h>
#include <vector>
#include <iostream>
#include <fstream>

class Network {
public:
    igraph_t graph;
    std::vector<long> weights;
    long edge_capacity; //one for all
    std::vector<long> source_indexes;
    std::string annotation;

    Network(std::string filename) {
        this->load(filename);
    }
    Network(igraph_t* g) {
        igraph_copy(&this->graph, g);
    }
    Network(igraph_t* g,
            std::vector<long>& weights,
            long edge_capacity,
            std::vector<long>& source_indexes) {
        igraph_copy(&this->graph, g);
        this->weights = weights;
        this->edge_capacity = edge_capacity;
        this->source_indexes = source_indexes;
    }
    ~Network() {
        igraph_destroy(&this->graph);
    }

    void save(std::string filename) {
        std::ofstream outf(filename,std::ios::app);
        outf << igraph_is_directed(&graph) << " "
             << igraph_vcount(&graph) << " "
             << igraph_ecount(&graph) << " "
             << edge_capacity << " "
             << source_indexes.size() << "\n";
        for (long i = 0; i < igraph_ecount(&graph); i++) {
            igraph_integer_t from, to;
            igraph_edge(&graph, i, &from, &to);
            outf << from << " " << to << " " << weights[i] << "\n";
        }
        for (long i = 0; i < source_indexes.size(); i++) {
            outf << source_indexes[i] << "\n";
        }
        outf << annotation;
        outf.close();
    }

    void load(std::string filename) {
        std::ifstream infile(filename, std::ios::in);
        if (!infile) {
            throw "File does not exist";
        }
        int is_directed;
        long vcount, ecount, source_num;
        infile >> is_directed >> vcount >> ecount >> edge_capacity >> source_num;
        igraph_empty(&graph, vcount, is_directed);
        weights.clear();
        for (long i = 0; i < ecount; i++) {
            long from, to, weight, capacity;
            infile >> from >> to >> weight;
            igraph_add_edge(&this->graph, from, to);
            weights.push_back(weight);
        }
        source_indexes.clear();
        for (long i = 0; i < source_num; i++) {
            long source_id;
            infile >> source_id;
            source_indexes.push_back(source_id);
        }
        annotation = "";
        while(!infile.eof()) {
            std::string additional_info;
            infile >> additional_info;
            annotation = annotation + " " + additional_info;
        }
    }
};

#endif //FCLA_NETWORK_H
