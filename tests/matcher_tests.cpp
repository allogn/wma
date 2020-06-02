//
// Created by allogn on 24.01.17.
//

#define BOOST_TEST_MODULE fcla_tests
#define BOOST_TEST_DYN_LINK
#define DBL_EPSILON 2.2204460492503131e-16

#include <boost/test/unit_test.hpp>
#include <igraph/igraph.h>
#include <iostream>
#include <Matcher.h>
#include <algorithm>
#include <time.h>
#include <EdgeGenerator.h>
#include <lemon/list_graph.h>
#include <lemon/cost_scaling.h>
#include "helpers.h"
#include "ExploringEdgeGenerator.h"
#include "Network.h"

using namespace std;

BOOST_AUTO_TEST_CASE (testSymmetricMatching) {
    int counter = 20;
    while(counter++ < 3) {
        long half_size = counter+10;
        RandomEdgeGenerator* egg = new RandomEdgeGenerator(half_size, half_size, half_size, 1);

        //init capacities (excess)
        std::vector<long> node_excess(half_size*2, 1);
        for (long i = 0; i< half_size; i++) {
            node_excess[i] = -1;
        }

        Logger logger;
        Matcher<long,long,long> M(egg, node_excess, &logger);
        M.match();
        M.calculateResult();

        //build igraph based on edges in EdgeGenerator and transform minmatch to maxmatch problem
        igraph_t test_graph;
        igraph_vector_bool_t types;
        igraph_vector_bool_init(&types, half_size*2); //used for igraph_matching
        igraph_vector_bool_fill(&types, true);
        for (long i = 0; i < half_size; i++) {
            VECTOR(types)[i] = false;
        }
        igraph_vector_t weights;
        igraph_vector_init(&weights,0);
        igraph_empty(&test_graph, half_size*2, true);
        egg->makeComplete();
        for (long i = 0; i < egg->edgeMemory.size(); i++) {
            newEdge e = egg->edgeMemory[i];
            igraph_add_edge(&test_graph, e.source_node, e.target_node);
            igraph_vector_push_back(&weights, e.weight);
        }
        //reverse and lift weights
        long max_w = igraph_vector_max(&weights);
        for (long i = 0; i < igraph_vector_size(&weights); i++) {
            VECTOR(weights)[i] = -VECTOR(weights)[i] + max_w;
        }

        //calculate and check matching
        igraph_real_t matching_weight;
        igraph_vector_long_t matching;
        igraph_vector_long_init(&matching, 0);
        igraph_maximum_bipartite_matching(&test_graph, &types, NULL, &matching_weight, &matching, &weights, DBL_EPSILON);
        BOOST_REQUIRE_EQUAL(- matching_weight + half_size*max_w, M.result_weight);

        delete egg;
        igraph_vector_destroy(&weights);
        igraph_vector_bool_destroy(&types);
        igraph_destroy(&test_graph);
        igraph_vector_long_destroy(&matching);
    }
}

long testFlowMatchingSD(EdgeGenerator* egg, long totalFlow, std::vector<long>& source_capacities, long target_capacity) {
    //create lemon graph and solve mincost
    lemon::ListDigraph g;
    lemon::ListDigraph::ArcMap<long> capacities(g);
    lemon::ListDigraph::ArcMap<long> weights(g);
    egg->makeLemon(&g, &capacities, &weights);
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
            capacities[e] = source_capacities[g.id(nodeit)];
            weights[e] = 0;
        } else {
            lemon::ListDigraph::Arc e = g.addArc(nodeit, target);
            capacities[e] = target_capacity;
            weights[e] = 0;
        }
    }
    lemon::CostScaling<lemon::ListDigraph, long, long> cost_scaling_alg(g);
    cost_scaling_alg.upperMap(capacities);
    cost_scaling_alg.costMap(weights);
    cost_scaling_alg.stSupply(source, target, totalFlow);
//        print_graph(&g, &weights, &capacities);
    lemon::CostScaling<lemon::ListDigraph, long, long>::ProblemType pt = cost_scaling_alg.run();
    if (pt != lemon::CostScaling<lemon::ListDigraph, long, long>::ProblemType::OPTIMAL) {
        throw "Cost Scaling graph is infeasible";
    }

    return cost_scaling_alg.totalCost();
}

BOOST_AUTO_TEST_CASE (multicapacityTest) {
    long source_n = 23;
    long target_n = 75;
    long target_capacity = 2;
    long source_capacity = 1;
    long total_size = source_n + target_n;
//        LoadedEdgeGenerator* egg = new LoadedEdgeGenerator("/Users/alvis/PhD/fcla/bin/egg.txt");
    RandomEdgeGenerator *egg = new RandomEdgeGenerator(source_n, source_n, target_n, target_capacity);

    std::vector<long> node_excess(total_size, target_capacity);
    for (long i = 0; i < source_n; i++) {
        node_excess[i] = -1;
    }

    Logger logger;
    Matcher<long,long,long> M(egg, node_excess, &logger);
    M.match();
    M.calculateResult();

    //calculate resulting cost of matching
    long totalFlow = min(source_n * source_capacity, target_n * target_capacity);
    std::vector<long> source_capacities(source_n, source_capacity);
    long testMatching = testFlowMatchingSD(egg, totalFlow, source_capacities, target_capacity);

    //compare results
    BOOST_REQUIRE_EQUAL(testMatching, M.result_weight);

    //test source capacity increase
    for (int i = 0; i < 1; i++) {
        M.increaseCapacity(i % source_n);
        source_capacities[i % source_n] += 1;
        M.calculateResult();
        totalFlow += 1;
        //note that all customers MUST be assigned, so there should be strongly greater number of services
        BOOST_CHECK_EQUAL(testFlowMatchingSD(egg, totalFlow, source_capacities, target_capacity), M.result_weight);
    }
}