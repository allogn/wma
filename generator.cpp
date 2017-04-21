//
// Created by Alvis Logins on 17/02/2017.
//

#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <igraph/igraph.h>
#include <time.h>

#include "helpers.h"
#include "Network.h"

using namespace std;
namespace po = boost::program_options;

void create_cluster(std::vector<double>& x, std::vector<double>& y, long n, double std, double center_x, double center_y) {
    igraph_rng_t rng_x;
    igraph_rng_t rng_y;
    igraph_rng_init(&rng_x, &igraph_rngtype_mt19937);
    igraph_rng_seed(&rng_x, time(NULL));
    igraph_rng_init(&rng_y, &igraph_rngtype_mt19937);
    igraph_rng_seed(&rng_y, time(NULL)/2);
    for (long i = 0; i < n; i++) {
        igraph_real_t x_point;
        igraph_real_t y_point;
        x_point = -1;
        y_point = -1;
        while (x_point < 0 || x_point > 1) {
            x_point = igraph_rng_get_normal(&rng_x, center_x, std);
        }
        while (y_point < 0 || y_point > 1) {
            y_point = igraph_rng_get_normal(&rng_y, center_y, std);
        }
        x.push_back(x_point);
        y.push_back(y_point);
    }
}

int main(int argc, const char** argv) {
    enum GRAPH_TYPE {
        GEOMETRIC, //random geometric with set of
        CLUSTERED,
        CLIQUES, //a set of connected cliques
        POWER, //power distributed graphs
        REAL //a subset of a city graph
    };

    // parsing parameters
    int graph_type;
    std::string outdir;
    long n;
    long sources;
    double geom_dens;
    long clusters;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("graph,g", po::value<int>(&graph_type)->required(), "Graph type")
            ("output,o", po::value<string>(&outdir)->required(), "Output directory")
            ("nodes,n", po::value<long>(&n)->required(), "Size of a graph")
            ("density,d", po::value<double>(&geom_dens)->default_value(1), "Density of a geometric graph, relatively to size")
            ("clusters,c", po::value<long>(&clusters)->default_value(1), "Number of clusters")
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
    std::vector<double> coord_x;
    std::vector<double> coord_y;
    std::vector<long> edges;
    igraph_rng_t rng_x;
    igraph_rng_t rng_y;
    igraph_bool_t check_connected;
    switch (graph_type) {
        /*
         * Density is a distance between connected nodes in 1x1 square
         */
        case GEOMETRIC:
            igraph_vector_t x;
            igraph_vector_t y;
            generate_random_geometric_graph(n, 1./sqrt((double)n)*geom_dens, &graph, weights, &x, &y);
            igraph_vector_destroy(&x);
            igraph_vector_destroy(&y);
            break;
        case CLUSTERED:
            //create a list of points and then connect close ones
            igraph_rng_init(&rng_x, &igraph_rngtype_mt19937);
            igraph_rng_seed(&rng_x, time(NULL));
            igraph_rng_init(&rng_y, &igraph_rngtype_mt19937);
            igraph_rng_seed(&rng_y, time(NULL)/2);
            check_connected = false;
            while (!check_connected) {
                coord_x.clear();
                coord_y.clear();
                edges.clear();
                weights.clear();
                std::vector<long> prev_center_id;
                for (long cl = 0; cl < clusters; cl++) {
                    igraph_real_t x_center;
                    igraph_real_t y_center;
                    x_center = igraph_rng_get_unif01(&rng_x);
                    y_center = igraph_rng_get_unif01(&rng_y);
                    create_cluster(coord_x, coord_y, n/clusters, 1./clusters, x_center, y_center);
                    prev_center_id.push_back(coord_x.size());
                    coord_x.push_back(x_center);
                    coord_y.push_back(y_center);
                }

                //create graph and add all edges with weights. weights multiply by 1000
                igraph_empty(&graph, coord_x.size(), false);
                for (long i = 0; i < coord_x.size(); i++) {
                    for (long j = i+1; j < coord_x.size(); j++) {
                        double dist = sqrt( pow(coord_x[i] - coord_x[j], 2) + pow(coord_y[i] - coord_y[j], 2));
                        if (dist < 1./sqrt((double)coord_x.size())*geom_dens) {
                            weights.push_back(dist * 1000);
                            edges.push_back(i);
                            edges.push_back(j);
                        }
                    }
                }
                //connect centers
                for (long i = 0; i < prev_center_id.size(); i++) {
                    for (long j = i+1; j < prev_center_id.size(); j++) {
                        double prev_center_x_1 = coord_x[prev_center_id[i]];
                        double prev_center_x_2 = coord_x[prev_center_id[j]];
                        double prev_center_y_1 = coord_y[prev_center_id[i]];
                        double prev_center_y_2 = coord_y[prev_center_id[j]];
                        double dist = sqrt( pow(prev_center_x_1 - prev_center_x_2, 2) + pow(prev_center_y_1 - prev_center_y_2, 2));
                        weights.push_back(dist * 1000);
                        edges.push_back(prev_center_id[i]);
                        edges.push_back(prev_center_id[j]);
                    }
                }
                //for point plotting
//                for (long i = 0; i < coord_x.size(); i++) {
//                    cout << coord_x[i] << "," << coord_y[i] << endl;
//                }
                create_graph(&graph, coord_x.size(), edges);
                igraph_is_connected(&(graph),&check_connected,IGRAPH_WEAK);
                if (!check_connected) {
                    igraph_destroy(&graph);
                }
            }
            break;
        default:
            throw "Method is not implemented";
    }

    //choose source_indexes randomly
    std::vector<long> all_nodes(n);
    for (long i = 0; i < n; i++) all_nodes[i] = i;
    std::random_shuffle(all_nodes.begin(),all_nodes.end());
    std::vector<long> source_indexes(sources);
    for (long i = 0; i < sources; i++) source_indexes[i] = all_nodes[i];

    Network network(&graph, weights, source_indexes);
    network.save(outdir);
}
