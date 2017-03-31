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
#include <time.h>

class Network {
public:
    igraph_t graph;
    std::string id;
    std::vector<long> weights;
    std::vector<long> source_indexes;

    Network(std::string filename) {
        this->load(filename);
    }
    Network(igraph_t* g,
            std::vector<long>& weights,
            std::vector<long>& source_indexes) {
        igraph_copy(&this->graph, g);
        this->weights = weights;
        this->source_indexes = source_indexes;

        //generate unique id
        struct timespec spec;
        clock_gettime(CLOCK_REALTIME, &spec);
        srand (spec.tv_nsec);
        std::string strtime = std::to_string(spec.tv_sec) + std::to_string(spec.tv_nsec);
        this->id = strtime.substr(0, strtime.size()-3) + std::to_string(rand() % 1000);
    }
    ~Network() {
        igraph_destroy(&this->graph);
    }

    void save(std::string dir, std::string filename) {
        std::ofstream outf(filename,std::ios::out);
        outf << this->id << " "
             << igraph_vcount(&graph) << " "
             << igraph_ecount(&graph) << " "
             << source_indexes.size() << "\n";
        for (long i = 0; i < igraph_ecount(&graph); i++) {
            igraph_integer_t from, to;
            igraph_edge(&graph, i, &from, &to);
            outf << from << " " << to << " " << weights[i] << "\n";
        }
        for (long i = 0; i < source_indexes.size(); i++) {
            outf << source_indexes[i] << "\n";
        }
        outf.close();
    }

    void save(std::string dir) {
        std::string filename = dir + '/' + this->id + ".ntw";
        this->save(dir, filename);
    }

    void load(std::string filename) {
        std::ifstream infile(filename, std::ios::in);
        if (!infile) {
            throw std::string("Input file does not exist");
        }
        long vcount, ecount, source_num;
        infile >> this->id >> vcount >> ecount >> source_num;
        igraph_empty(&graph, vcount, false);
        igraph_vector_t edges;
        igraph_vector_init(&edges, ecount*2);
        weights.clear();
        weights.reserve(ecount);
        for (long i = 0; i < ecount; i++) {
            long from, to, weight, capacity;
            infile >> from >> to >> weight;
            VECTOR(edges)[2*i] = from;
            VECTOR(edges)[2*i + 1] = to;
            weights.push_back(weight);
        }
        igraph_add_edges(&this->graph, &edges, 0);
        source_indexes.clear();
        source_indexes.reserve(source_num);
        for (long i = 0; i < source_num; i++) {
            long source_id;
            infile >> source_id;
            source_indexes.push_back(source_id);
        }

        //check if consistent
//        for (long i = 0; i < ecount; i++) {
//            igraph_integer_t from;
//            igraph_integer_t to;
//            igraph_edge(&graph, i, &from, &to);
//            if ((from != VECTOR(edges)[2*i]) || (to != VECTOR(edges)[2*i+1])) {
//                throw "not valid";
//            }
//        }
        igraph_vector_destroy(&edges);
    }
};

#endif //FCLA_NETWORK_H
