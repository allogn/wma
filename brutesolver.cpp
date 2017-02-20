//
// Created by Alvis Logins on 20/02/2017.
//


#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <igraph/igraph.h>
#include <lemon/list_graph.h>
#include <lemon/cost_scaling.h>

#include "helpers.h"
#include "Network.h"
#include "ExploringEdgeGenerator.h"

using namespace std;
namespace po = boost::program_options;

long calculate_objective(Network& network,
                         igraph_matrix_t* shortest_paths_matx,
                         std::vector<long>& facility_indexes,
                         long facility_capacity)
{
    lemon::ListDigraph g;
    lemon::ListDigraph::ArcMap<long> capacities(g);
    lemon::ListDigraph::ArcMap<long> weights(g);

    // add nodes
    std::vector<lemon::ListDigraph::Node> nodes;
    long vcount = facility_indexes.size() + network.source_indexes.size(); //bipartite graph
    for (long i = 0; i < vcount; i++) {
        lemon::ListDigraph::Node node = g.addNode();
        nodes.push_back(node);
    }
    for (long i = 0; i < network.source_indexes.size(); i++) {
        for (long j = 0; j < facility_indexes.size(); j++) {
            lemon::ListDigraph::Arc e = g.addArc(nodes[i], nodes[network.source_indexes.size()+j]);
            capacities[e] = 1;
            weights[e] = MATRIX((*shortest_paths_matx), network.source_indexes[i], facility_indexes[j]);
        }
    }

    //add additional source and destination nodes
    lemon::ListDigraph::Node source = g.addNode();
    lemon::ListDigraph::Node target = g.addNode();
    //if a node has at least one outgoing edge, then this node is a source node
    for (lemon::ListDigraph::NodeIt nodeit(g); nodeit != lemon::INVALID; ++nodeit) {
        if ((g.id(nodeit) == g.id(source)) || (g.id(nodeit) == g.id(target))) continue;
        lemon::ListDigraph::OutArcIt eit(g, nodeit);
        if (eit != lemon::INVALID) {
            //this is source node
            lemon::ListDigraph::Arc e = g.addArc(source, nodeit);
            capacities[e] = 1;
            weights[e] = 0;
        } else {
            lemon::ListDigraph::Arc e = g.addArc(nodeit, target);
            capacities[e] = facility_capacity;
            weights[e] = 0;
        }
    }
    lemon::CostScaling<lemon::ListDigraph, long, long> cost_scaling_alg(g);
    cost_scaling_alg.upperMap(capacities);
    cost_scaling_alg.costMap(weights);
    cost_scaling_alg.stSupply(source, target, network.source_indexes.size());
    //        print_graph(&g, &weights, &capacities);
    lemon::CostScaling<lemon::ListDigraph, long, long>::ProblemType pt = cost_scaling_alg.run();
    if (pt != lemon::CostScaling<lemon::ListDigraph, long, long>::ProblemType::OPTIMAL) {
        throw "Cost Scaling graph is infeasible";
    }

    return cost_scaling_alg.totalCost();
}

//return false if finished
bool next_facility_indexes(std::vector<long>& facility_index, long max_index, long cur) {
    /*
     * < 0, 1, 2 >
     * < 0 - (max-2), 1 - (max-1), 2 - max>
     */
    long size = facility_index.size();
    facility_index[cur]++;
    if (facility_index[cur] == max_index) {
        if (cur == 0) return false;
        bool result = next_facility_indexes(facility_index, max_index, cur-1);
        facility_index[cur] = facility_index[cur-1];
        return result;
    }
    return true;
}

int main(int argc, const char** argv) {
    string filename;
    long facilities_to_locate;
    long facility_capacity;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("input,i", po::value<string>(&filename)->required(), "Input file, a network")
            ("facilities,n", po::value<long>(&facilities_to_locate)->required(), "Facilities to locate")
            ("faccap,c", po::value<long>(&facility_capacity)->default_value(1), "Capacity of facilities");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    po::notify(vm);

    Network network(filename);

    // calculate shortest paths
    igraph_vector_t real_weights;
    igraph_vector_init(&real_weights, igraph_ecount(&network.graph));
    for (long i = 0 ; i < network.weights.size(); i++)
        VECTOR(real_weights)[i] = network.weights[i];

    igraph_vs_t all_nodes;
    igraph_vs_all(&all_nodes);
    igraph_matrix_t res_matx;
    igraph_matrix_init(&res_matx, 0, 0);
    igraph_shortest_paths_bellman_ford(&network.graph, &res_matx, all_nodes, all_nodes, &real_weights, IGRAPH_ALL);
    igraph_vector_destroy(&real_weights);

    //for each set of facilities - calculate matching (objective function)
    long best_objective = LONG_MAX;
    std::vector<long> facility_index(facilities_to_locate,0);
    for (long i = 0; i < facilities_to_locate; i++) {
        facility_index[i] = i;
    }
    long max_index = igraph_vcount(&network.graph) - 1;
    do {
        best_objective = std::min(best_objective, calculate_objective(network, &res_matx,
                                                                      facility_index, facility_capacity));
    } while (next_facility_indexes(facility_index, max_index, facilities_to_locate-1));

    cout << best_objective << endl;

}