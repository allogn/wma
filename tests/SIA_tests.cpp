//
// Created by allogn on 24.01.17.
//

#define BOOST_TEST_MODULE SIA_tests
#define DBL_EPSILON 2.2204460492503131e-16

#include <boost/test/included/unit_test.hpp>
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

//BOOST_AUTO_TEST_CASE (dijkstra_random_test) {
//
//    //generate geometric random graph and test dijsktra
//    igraph_t undir_graph;
//    igraph_vector_t x; //random points in geometric graph
//    igraph_vector_t y;
//    int size = 500;
//
//    Matcher<int,long,int> M(size);
//    generate_random_geometric_graph(size, 0.1, &undir_graph, M.weights, &x, &y);
//    make_directed(&undir_graph, &M.graph);
//
//    //find path from left to right
//    igraph_integer_t min_ind = static_cast<igraph_integer_t>(igraph_vector_which_min(&x));
//    igraph_integer_t max_ind = static_cast<igraph_integer_t>(igraph_vector_which_max(&x));
//
//    M.node_excess.resize(M.graph_size, -1);
//    M.node_excess[max_ind] = 1;
//    M.edge_excess.resize(igraph_ecount(&M.graph),1);
//
//    M.new_edges.resize(M.graph_size);
//    for (int i = 0; i < M.graph_size; i++) M.new_edges[i].exists = false;
//    M.potentials.resize(M.graph_size, 0);
//    igraph_vector_long_t potentials;
//    M.iteration_init(min_ind);
//
//    long closest_target = M.dijkstra();
//
//    //check with igraph dijkstra
//    igraph_matrix_t correct_answer;
//    igraph_matrix_init(&correct_answer, 0, 0);
//    igraph_vector_t real_weights;
//    igraph_integer_t wsize = igraph_ecount(&M.graph);
//    igraph_vector_init(&real_weights, wsize);
//    for (igraph_integer_t i = 0; i < wsize; i++)
//        igraph_vector_set(&real_weights, i, M.weights[i]);
//    BOOST_REQUIRE_EQUAL(igraph_vector_min(&real_weights), *std::min_element(M.weights.begin(),M.weights.end()));
//    igraph_shortest_paths_dijkstra(&M.graph, &correct_answer, igraph_vss_1(min_ind),
//                                   igraph_vss_1(max_ind), &real_weights, IGRAPH_OUT);
//    BOOST_REQUIRE_EQUAL((closest_target == -1), (MATRIX(correct_answer, 0, 0) == IGRAPH_INFINITY));
//    if (closest_target != -1)
//        BOOST_CHECK_EQUAL(MATRIX(correct_answer, 0, 0), M.mindist[closest_target]);
//    //@todo check memleak when valgrind is available for macOS12
//
//    //free memory
//    igraph_vector_destroy(&real_weights);
//    igraph_matrix_destroy(&correct_answer);
//}

//BOOST_AUTO_TEST_CASE (dijkstra_with_edge_addition) {
//    unsigned int seed = time(NULL);
//    for (int i = 0; i < 3; i++) {
//        //build random clique with edge addition and check Dijkstra at each iteration
//        int size = 20;
//        Matcher<int,int,int> M(size);
//
//        //generate edge sequence
//        newEdges new_edges;
//        srand(++seed);
//        for (int i = 0; i < size; i++) {
//            for (int j = i+1; j < size; j++) {
//                int ind = i*size + j;
//                newEdge new_edge;
//                new_edge.exists = true;
//                new_edge.weight = rand() % 200;
//                new_edge.source_node = i;
//                new_edge.target_node = j;
//                new_edge.capacity = 1;
//                new_edges.push_back(new_edge);
//            }
//        }
//        std::random_shuffle(new_edges.begin(), new_edges.end());
//
//        igraph_integer_t source = 0;
//        igraph_integer_t target = size-1;
//
//        M.node_excess.resize(M.graph_size, -1);
//        M.node_excess[target] = 1;
//
//        M.potentials.resize(M.graph_size, 0);
//        M.iteration_init(source);
//        M.empty_new_edges();
//
//        igraph_integer_t closest_target;
//
//        //while graph is not clique
//        for (int i = 0; i < size*(size-1)/2; i++) {
//            //add edge and run Dijsktras
//            igraph_integer_t old_ecount = igraph_ecount(&M.graph);
//            M.addNewEdge(new_edges[i]);
//            BOOST_CHECK_EQUAL(old_ecount, igraph_ecount(&M.graph)-2);
//            igraph_integer_t from, to;
//            igraph_edge(&M.graph, old_ecount, &from, &to);
//            BOOST_CHECK_EQUAL(from, new_edges[i].source_node);
//            BOOST_CHECK_EQUAL(to, new_edges[i].target_node);
//            //run dijkstra
//            closest_target = M.dijkstra();
//
//            //check with igraph dijkstra
//            igraph_t non_neg_graph;
//            igraph_empty(&non_neg_graph, igraph_vcount(&M.graph), true);
//
//            igraph_vector_t real_weights;
//            igraph_vector_init(&real_weights, 0);
//
//            for (long i = 0; i < igraph_ecount(&M.graph); i++) {
//                if (M.weights[i] >= 0 && M.edge_excess[i] > 0) {
//                    igraph_integer_t from, to;
//                    igraph_edge(&M.graph, i, &from, &to);
//                    igraph_add_edge(&non_neg_graph, from, to);
//                    igraph_vector_push_back(&real_weights, M.weights[i]);
//                }
//            }
//
//            igraph_matrix_t correct_answer;
//            igraph_matrix_init(&correct_answer, 0, 0);
//            BOOST_REQUIRE_EQUAL(igraph_vector_max(&real_weights), *std::max_element(M.weights.begin(), M.weights.end()));
//            igraph_shortest_paths_dijkstra(&non_neg_graph, &correct_answer, igraph_vss_1(source),
//                                           igraph_vss_1(target), &real_weights, IGRAPH_OUT);
//            BOOST_REQUIRE_EQUAL((closest_target == -1), (MATRIX(correct_answer, 0, 0) == IGRAPH_INFINITY));
//            if (closest_target != -1) {
//                BOOST_REQUIRE_EQUAL(MATRIX(correct_answer, 0, 0), M.mindist[closest_target]);
//            }
//            igraph_vector_destroy(&real_weights);
//            igraph_destroy(&non_neg_graph);
//        }
//    }
//
//}

//BOOST_AUTO_TEST_CASE (FlowAndPotentialsTest) {
//    Matcher<int,int,int> M(5);
//
//    igraph_vector_t edges;
//    igraph_real_t edge_array[20] = {0,1,1,2,2,3,3,2,2,1,1,0,1,3,2,4,3,1,4,2};
//    igraph_vector_init_copy(&edges, edge_array, 20);
//    igraph_add_edges(&M.graph, &edges, NULL);
//    igraph_vector_destroy(&edges);
//
//    M.edge_excess = {9, 2, 9, 0, 0, 0, 10, 10, 10, 10};
//    M.node_excess = {-2, -2, 0, 2, 2};
//    M.backtrack = {0, 0, 1, 2, 2};
//    M.potentials = {0, 0, 0, 0, 0};
//    M.augmentFlow(3);
//    int answer[10] = {7, 0, 7, 2, 2, 2, 10, 10, 10, 10};
//    for (int i = 0; i < 10; i++) {
//        BOOST_CHECK_EQUAL(answer[i],M.edge_excess[i]);
//    }
//
//    M.mindist = {0, 3, 6, 9, 10};
//    M.updatePotentials(3);
//    long answer_potentials[5] = {9, 6, 3, 0, 0};
//    for (int i = 0; i < 5; i++) {
//        BOOST_CHECK_EQUAL(answer_potentials[i],M.potentials[i]);
//    }
//}


BOOST_AUTO_TEST_CASE (testMatching) {
    // test symetric matching for a set of random graphs using igraph matching
    int counter = 20;
    while(counter++ < 3) {
        long half_size = counter+10;
        RandomEdgeGenerator* egg = new RandomEdgeGenerator(half_size, half_size, half_size, 1);
//        LoadedEdgeGenerator* egg = new LoadedEdgeGenerator("/Users/alvis/PhD/fcla/bin/egg.txt");

        //init capacities (excess)
        std::vector<long> node_excess(half_size*2, 1);
        for (long i = 0; i< half_size; i++) {
            node_excess[i] = -1;
        }

        Logger logger;
        Matcher<long,long,long> M(egg, node_excess, &logger);
        M.run();
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
//        egg->save("egg.txt");
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
    M.run();
    M.calculateResult();
//        egg->save("egg.txt");

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
        //note that all customers MUST be assigned, so there should be strongly hreater number of services
        BOOST_CHECK_EQUAL(testFlowMatchingSD(egg, totalFlow, source_capacities, target_capacity), M.result_weight);
    }
}

