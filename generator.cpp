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

void create_cluster(std::vector<Coords>& coords, long n, double std, double center_x, double center_y) {
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
        coords.push_back(std::make_pair(x_point,y_point));
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
    bool connected;
    bool repeat;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("graph,g", po::value<int>(&graph_type)->required(), "Graph type")
            ("output,o", po::value<string>(&outdir)->required(), "Output directory")
            ("nodes,n", po::value<long>(&n)->required(), "Size of a graph")
            ("density,d", po::value<double>(&geom_dens)->default_value(1), "Density of a geometric graph, relatively to size")
	        ("connected,u", po::value<bool>(&connected)->default_value(false), "Force to have one component")
            ("clusters,c", po::value<long>(&clusters)->default_value(1), "Number of clusters")
            ("repeat,r", po::value<bool>(&repeat)->default_value(false), "Allow multiple customers per node")
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
    std::vector<Coords> coords;
    std::vector<long> edges;
    igraph_rng_t rng_x;
    igraph_rng_t rng_y;
    igraph_bool_t check_connected = false;
    while (!check_connected) {
   	    coords.clear();
	    coords.clear();
	    edges.clear();
	    weights.clear();

	    switch (graph_type) {
		/*
		 * Density is a distance between connected nodes in 1x1 square
		 */
		case GEOMETRIC:
		    igraph_vector_t x;
		    igraph_vector_t y;
		    generate_random_geometric_graph(n, 1./sqrt((double)n)*geom_dens, &graph, weights, &x, &y);
		    coords.clear();
		    for (long i = 0; i < n; i++) {
			coords.push_back(std::make_pair(VECTOR(x)[i],VECTOR(y)[i]));
		    }
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
		    {
		        std::vector<long> prev_center_id;
			for (long cl = 0; cl < clusters; cl++) {
			    igraph_real_t x_center;
			    igraph_real_t y_center;
			    x_center = igraph_rng_get_unif01(&rng_x);
			    y_center = igraph_rng_get_unif01(&rng_y);
			    create_cluster(coords, n / clusters, 1. / clusters, x_center, y_center);
			    prev_center_id.push_back(coords.size());
			    coords.push_back(std::make_pair(x_center,y_center));
			}

			//create graph and add all edges with weights. weights multiply by 1000
			igraph_empty(&graph, coords.size(), false);
			for (long i = 0; i < coords.size(); i++) {
			    for (long j = i + 1; j < coords.size(); j++) {
                    double dist = sqrt(pow(coords[i].first - coords[j].first, 2) + pow(coords[i].second - coords[j].second, 2));
                    if (dist < 1. / sqrt((double) coords.size()) * geom_dens) {
                        weights.push_back(dist * 1000);
                        edges.push_back(i);
                        edges.push_back(j);
                    }
			    }
			}
			//connect centers
			for (long i = 0; i < prev_center_id.size(); i++) {
			    for (long j = i + 1; j < prev_center_id.size(); j++) {
                    long node1 = prev_center_id[i];
                    long node2 = prev_center_id[j];
                    
                    bool exist = false;
                    //check if they are not yet connected
                    for (long k = 0; k < edges.size(); k+=2) {
                        if ((edges[k] == node1) && (edges[k+1] == node2)) {
                            exist = true;
                        }
                    }
                    if (exist) continue;
                    
                    double prev_center_x_1 = coords[node1].first;
                    double prev_center_x_2 = coords[node2].first;
                    double prev_center_y_1 = coords[node1].second;
                    double prev_center_y_2 = coords[node2].second;
                    double dist = sqrt(
                        pow(prev_center_x_1 - prev_center_x_2, 2) + pow(prev_center_y_1 - prev_center_y_2, 2));
                    weights.push_back(dist * 1000);
                    edges.push_back(node1);
                    edges.push_back(node2);
			    }
			}
			create_graph(&graph, coords.size(), edges);

            if (connected) {
                //connect all components one to the next one
                igraph_vector_t membership;
                igraph_vector_init(&membership, 0);
                igraph_vector_t csize;
                igraph_vector_init(&csize, 0);
                igraph_integer_t no;
                igraph_clusters(&graph, &membership, &csize, &no, IGRAPH_WEAK);
//                std::vector<long> new_edges;
//                std::vector<long> new_weights;
//                std::vector<long> component_representatives(no, -1);
//                if (no > 1) {
//                    for (igraph_integer_t member = 0; member < igraph_vcount(&graph); member++) {
//                        igraph_integer_t component_id = VECTOR(membership)[member];
//                        if (component_representatives[component_id] == -1) {
//                            component_representatives[component_id] = member;
//                        }
//                    }
//                    for (igraph_integer_t component_id = 0; component_id < no; component_id++) {
//                        //get first node in component1
//                        long node1 = component_representatives[component_id];
//                        //get first node in the next component (or first component if it is the last)
//                        long node2;
//                        if (component_id == no-1) {
//                            node2 = component_representatives[0];
//                        } else {
//                            node2 = component_representatives[component_id+1];
//                        }
//                        //connect two nodes
//                        double prev_center_x_1 = coords[node1].first;
//                        double prev_center_x_2 = coords[node2].first;
//                        double prev_center_y_1 = coords[node1].second;
//                        double prev_center_y_2 = coords[node2].second;
//                        double dist = sqrt(
//                                pow(prev_center_x_1 - prev_center_x_2, 2) + pow(prev_center_y_1 - prev_center_y_2, 2));
//                        weights.push_back(dist * 1000);
//                        new_edges.push_back(node1);
//                        new_edges.push_back(node2);
//                    }
//                }
//
//                igraph_vector_t v1;
//                igraph_real_t real_edges[new_edges.size()];
//                for (long i = 0; i < new_edges.size(); i++) {
//                    real_edges[i] = (double)new_edges[i];
//                }
//                igraph_vector_view(&v1, real_edges, sizeof(real_edges)/sizeof(double));
//                igraph_add_edges(&graph, &v1, 0);

                std::vector<long> sizes(no);
                for (long i = 0; i < no; i++) {
                    sizes[i] = VECTOR(csize)[i];
                }
                std::sort(sizes.begin(), sizes.end(), std::greater<long>());
                assert(sizes[0] >= sizes[sizes.size() - 1]);

                long members_id = -1;
                for (long i = 0; i < sizes.size(); i++) {
                    if (VECTOR(csize)[i] == sizes[0]) {
                        members_id = i;
                        break;
                    }
                }
                assert(members_id > -1);
                std::vector<double> vlist;
                for (long i = 0; i < igraph_vcount(&graph); i++) {
                    if (VECTOR(membership)[i] == members_id) {
                        vlist.push_back(i);
                    }
                }

                if (sizes[0] >= igraph_vcount(&graph)*(4./5.)) {
                    //remove small components
                    igraph_t new_graph;
                    igraph_vs_t vids;
                    igraph_vector_t vids_vec;
                    igraph_vector_init_copy(&vids_vec, &vlist[0], vlist.size());

                    igraph_vs_vector(&vids, &vids_vec);
                    igraph_induced_subgraph(&graph, &new_graph, vids, IGRAPH_SUBGRAPH_AUTO);

                    igraph_vector_destroy(&vids_vec);
                    igraph_vs_destroy(&vids);
                    igraph_destroy(&graph);
                    graph = new_graph;
                }

                igraph_vector_destroy(&membership);
                igraph_vector_destroy(&csize);
            }
		    }
		    break;
		default:
		    throw "Method is not implemented";
	    }
        if (!connected) {
          check_connected = true;
        } else {
            igraph_is_connected(&(graph),&check_connected,IGRAPH_WEAK);
            if (!check_connected) {
              igraph_destroy(&graph);
            }
        }
    }

    //choose source_indexes randomly
    std::vector<long> source_indexes;
    n = igraph_vcount(&graph);
    source_indexes.resize(sources);
    if (!repeat) {
        if (sources > n) {
            throw "Too many sources";
        }
        std::vector<long> all_nodes(n);
        for (long i = 0; i < n; i++) all_nodes[i] = i;
        std::random_shuffle(all_nodes.begin(),all_nodes.end());
        for (long i = 0; i < sources; i++) source_indexes[i] = all_nodes[i];
    } else {
        for (long i = 0; i < sources; i++) {
            source_indexes[i] = randint(0, n-1);
        }
    }

    Network network(&graph, weights, source_indexes, coords);
    network.save(outdir);
    igraph_destroy(&graph);
}
