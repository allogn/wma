//
// Created by Alvis Logins on 17/02/2017.
//

#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <igraph/igraph.h>

#include "helpers.h"
#include "Network.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, const char** argv) {

    enum GRAPH_TYPE {
        GEOMETRIC, //random geometric with set of
        CLIQUES, //a set of connected cliques
        POWER, //power distributed graphs
        REAL //a subset of a city graph
    };

    // parsing parameters
    int graph_type;
    string outf;
    long n;
    long sources;
    double geom_dens;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("graph,g", po::value<int>(&graph_type)->required(), "Graph type")
            ("output,o", po::value<string>(&outf)->required(), "Output file")
            ("nodes,n", po::value<long>(&n)->required(), "Size of a graph")
            ("density", po::value<double>(&geom_dens)->default_value(0.4), "Density of a geometric graph")
            ("sources,s", po::value<long>(&sources)->default_value(1), "Number of Sources (customers)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    po::notify(vm);

    igraph_t graph;
    vector<long> weights;
    switch (graph_type) {
        case GEOMETRIC:
            igraph_vector_t x;
            igraph_vector_t y;
            generate_random_geometric_graph(n, geom_dens, &graph, weights, &x, &y);
            igraph_vector_destroy(&x);
            igraph_vector_destroy(&y);
            break;
        default:
            throw "Method is not implemented";
    }

    //choose source_indexes randomly
    vector<long> all_nodes(n);
    for (long i = 0; i < n; i++) all_nodes[i] = i;
    std::random_shuffle(all_nodes.begin(),all_nodes.end());
    std::vector<long> source_indexes(sources);
    for (long i = 0; i < sources; i++) source_indexes[i] = all_nodes[i];

    Network network(&graph, weights, 1, source_indexes);
    network.save(outf);
    cout << "Done." << endl;
}