//
// Created by allogn on 17.01.17.
//

#ifndef FCLA_HELPERS_H
#define FCLA_HELPERS_H

#include <string>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <igraph/igraph.h>
#include <lemon/list_graph.h>

#define WEIGHT_ERROR 0.001

// check if file exists
inline bool file_exists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

typedef std::pair<float, float> coords;


/*
 * transform double weight into integer weight according to macro error
 * assuming scale of double weights from 0 to 1.
 */
inline long intWeight(double weight) { return static_cast<long>(weight/WEIGHT_ERROR); }
inline long intWeightFromCoords(double x1, double y1, double x2, double y2) {
    return intWeight(sqrt(pow(x2-x1, 2) + pow(y2-y1, 2)));
}


//@todo not tested, used at least in SIA tests
//input: uninitialized graph, pointer to uninitialized weights, size of the graph
int generate_random_geometric_graph(long size,
                                    double density, //from 0 to 1
                                    igraph_t* result_graph,
                                    std::vector<long>& weights,
                                    igraph_vector_t* x,
                                    igraph_vector_t* y)
{
    igraph_vector_init(x, size);
    igraph_vector_init(y, size);
    int res = igraph_grg_game(result_graph, size, density, false, x, y); //false goes for square area instead of torus
    //calculate integer weights
    igraph_integer_t ec = igraph_ecount(result_graph);
    weights.resize(ec);
    for (igraph_integer_t i = 0; i < ec; i++) {
        igraph_integer_t from, to;
        igraph_edge(result_graph, i, &from, &to);
        weights[i] = intWeightFromCoords(VECTOR(*x)[from], VECTOR(*y)[from], VECTOR(*x)[to], VECTOR(*y)[to]);
    }
    return res; //error code from igraph generator
}

/*
 * Make directed graph, input initialized
 */
void make_directed(igraph_t* undirected, igraph_t* directed) {
    for (igraph_integer_t i = 0; i < igraph_ecount(undirected); i++) {
        igraph_integer_t from;
        igraph_integer_t to;
        igraph_edge(undirected, i, &from, &to);
        igraph_add_edge(directed, from, to);
    }
}

std::string edge_str(igraph_t* graph, igraph_vector_long_t* weights, igraph_vector_long_t* excess, igraph_integer_t eid) {
    std::ostringstream res;
    igraph_integer_t from;
    igraph_integer_t to;
    igraph_edge(graph, eid, &from, &to);
    res << from << " - " << to << "(" << VECTOR(*weights)[eid] << "," << VECTOR(*excess)[eid] <<")";
    return res.str();
}

void print_graph(igraph_t* graph, igraph_vector_long_t* weights, igraph_vector_long_t* excess) {
    std::cout << "-----------" << std::endl;
    if (igraph_is_directed(graph)) {
        std::cout << "Directed graph" << std::endl;
    } else {
        std::cout << "Undirected graph" << std::endl;
    }
    std::cout << "Nodes: " << igraph_vcount(graph) << "; Edges: " << igraph_ecount(graph) << std::endl;
    for (long i = 0; i < igraph_ecount(graph); i++) {
        std::cout << edge_str(graph, weights, excess, i) << std::endl;
    }
    std::cout << "Node incidents: " << std::endl;
    igraph_vector_t eids;
    igraph_vector_init(&eids, 0);
    for (long i = 0; i < igraph_vcount(graph); i++) {
        std::cout << i << ": ";
        if (igraph_is_directed(graph)) {
            std::cout << "OUT: ";
            igraph_incident(graph, &eids, i, IGRAPH_OUT);
            for (long j = 0; j < igraph_vector_size(&eids); j++) {
                std::cout << edge_str(graph, weights, excess, VECTOR(eids)[j]) << ",";
            }
            std::cout << "; IN: ";
            igraph_incident(graph, &eids, i, IGRAPH_IN);
            for (long j = 0; j < igraph_vector_size(&eids); j++) {
                std::cout << edge_str(graph, weights, excess, VECTOR(eids)[j]) << ",";
            }
            std::cout << "\n";
        } else {
            igraph_incident(graph, &eids, i, IGRAPH_ALL);
            for (long j = 0; j < igraph_vector_size(&eids); j++) {
                std::cout << edge_str(graph, weights, excess, VECTOR(eids)[j]) << ",";
            }
            std::cout << "\n";
        }
    }
    igraph_vector_destroy(&eids);
    std::cout << "-----------\n\n";
}

void print_graph(lemon::ListDigraph* g,
                 lemon::ListDigraph::ArcMap<long>* weights,
                 lemon::ListDigraph::ArcMap<long>* edge_excess) {
    std::cout << "-----------" << std::endl;
    for (lemon::ListDigraph::NodeIt nit(*g); nit != lemon::INVALID; ++nit) {
        std::cout << g->id(nit) << ": ";
        std::cout << "OUT: ";
        for (lemon::ListDigraph::OutArcIt eit(*g, nit); eit != lemon::INVALID; ++eit) {
            std::cout << g->id(g->source(eit)) << "->" << g->id(g->target(eit))
                      << "(" << (*weights)[eit] << "," << (*edge_excess)[eit] << "),";
        }
        std::cout << ";IN: ";
        for (lemon::ListDigraph::InArcIt eit(*g, nit); eit != lemon::INVALID; ++eit) {
            std::cout << g->id(g->source(eit)) << "->" << g->id(g->target(eit))
                      << "(" << (*weights)[eit] << "," << (*edge_excess)[eit] << "),";
        }
        std::cout << std::endl;
    }
    std::cout << "-----------\n\n";
}

#endif //FCLA_HELPERS_H
