//
// Created by allogn on 24.01.17.
//

#define BOOST_TEST_MODULE SIA_tests
#include <boost/test/included/unit_test.hpp>
#include <igraph/igraph.h>
#include <iostream>
#include <SIA.h>
#include <algorithm>
#include <time.h>
#include <EdgeGenerator.h>
#include "helpers.h"
#include "Bipartite.h"

using namespace SIA;
using namespace std;

BOOST_AUTO_TEST_CASE (dijsktra_simple_test) {
    //primitive graph
    igraph_t graph;
    igraph_empty(&graph, 2, true);
    igraph_add_edge(&graph, 0, 1);
    igraph_vector_t real_weight;
    igraph_vector_init(&real_weight, 1);
    igraph_vector_set(&real_weight, 0, 10);
    igraph_vector_long_t long_weight;
    igraph_vector_long_init(&long_weight, 1);
    igraph_vector_long_set(&long_weight, 0, 10);
    igraph_vector_long_t node_excess;
    igraph_vector_long_init(&node_excess, 2);
    igraph_vector_long_fill(&node_excess, -1);
    igraph_vector_long_set(&node_excess, 1, 1);
    igraph_vector_long_t excess;
    igraph_vector_long_init(&excess, 1);
    igraph_vector_long_fill(&excess, 1);
    igraph_vector_long_t potentials;
    igraph_vector_long_init(&potentials, 2);
    igraph_vector_long_fill(&potentials, 0);

    mmHeap dheap; //heap used in Dijkstra
    mmHeap gheap; //dummy for dijkstra
    newEdges next_edges; //dummy for Dijkstra, must contain non-existing edges
    next_edges.resize(2);
    for (int i = 0; i < 2; i++) next_edges[i].exists = false;
    igraph_vector_long_t mindist; //minimum distance in Dijstra execution
    igraph_vector_t backtrack; //ids of ancestor in Dijkstra tree
    init_dijsktra(2, &dheap, &mindist, &backtrack, 0);

    igraph_integer_t closest_target;
    closest_target = dijkstra(&graph, &dheap, &gheap, next_edges, &mindist, &excess, &node_excess, &potentials, &long_weight, &backtrack);
    BOOST_CHECK(closest_target == 1);

    igraph_matrix_t correct_answer;
    igraph_matrix_init(&correct_answer, 0, 0);
    igraph_shortest_paths_dijkstra(&graph, &correct_answer, igraph_vss_1(0), igraph_vss_all(), &real_weight, IGRAPH_OUT);
    BOOST_CHECK(MATRIX(correct_answer, 0, 0) == 0);
    BOOST_CHECK(MATRIX(correct_answer, 0, 1) == 10);
    igraph_shortest_paths_dijkstra(&graph, &correct_answer, igraph_vss_1(1), igraph_vss_all(), &real_weight, IGRAPH_OUT);
    BOOST_CHECK(MATRIX(correct_answer, 0, 0) == IGRAPH_INFINITY);
    BOOST_CHECK(MATRIX(correct_answer, 0, 1) == 0);

    //free memory
    igraph_vector_destroy(&real_weight);
    igraph_vector_long_destroy(&long_weight);
    igraph_vector_long_destroy(&node_excess);
    igraph_vector_destroy(&backtrack);
    igraph_vector_long_destroy(&excess);
    igraph_matrix_destroy(&correct_answer);
    igraph_destroy(&graph);
}

BOOST_AUTO_TEST_CASE (dijkstra_random_test) {

    //generate geometric random graph and test dijsktra
    igraph_t undir_graph;
    igraph_t graph;
    igraph_vector_t x; //random points in geometric graph
    igraph_vector_t y;
    igraph_vector_long_t weights;
    long size = 500;
    generate_random_geometric_graph(size, 0.1, &undir_graph, &weights, &x, &y);
    make_directed(&undir_graph, &graph);

    igraph_vector_long_t excess;
    igraph_vector_long_init(&excess, igraph_ecount(&graph));
    igraph_vector_long_fill(&excess, 1);

    //find path from left to right
    igraph_integer_t min_ind = static_cast<igraph_integer_t>(igraph_vector_which_min(&x));
    igraph_integer_t max_ind = static_cast<igraph_integer_t>(igraph_vector_which_max(&x));

    igraph_vector_long_t node_excess;
    igraph_vector_long_init(&node_excess, size);
    igraph_vector_long_fill(&node_excess, -1);
    igraph_vector_long_set(&node_excess, max_ind, 1);

    mmHeap dheap; //heap used in Dijkstra
    mmHeap gheap; //dummy for dijkstra
    newEdges next_edges; //dummy for Dijkstra, must contain non-existing edges
    next_edges.resize(size);
    for (int i = 0; i < size; i++) next_edges[i].exists = false;
    igraph_vector_long_t mindist; //minimum distance in Dijstra execution
    igraph_vector_long_t potentials;
    igraph_vector_long_init(&potentials, size);
    igraph_vector_long_fill(&potentials, 0);
    igraph_vector_t backtrack; //ids of ancestor in Dijkstra tree
    init_dijsktra(size, &dheap, &mindist, &backtrack, min_ind);

    igraph_integer_t closest_target;
    closest_target = dijkstra(&graph, &dheap, &gheap, next_edges, &mindist, &excess, &node_excess, &potentials, &weights, &backtrack);

    //check with igraph dijkstra
    igraph_matrix_t correct_answer;
    igraph_matrix_init(&correct_answer, 0, 0);
    igraph_vector_t real_weights;
    igraph_integer_t wsize = igraph_vector_long_size(&weights);
    igraph_vector_init(&real_weights, wsize);
    for (igraph_integer_t i = 0; i < wsize; i++)
        igraph_vector_set(&real_weights, i, VECTOR(weights)[i]);
    BOOST_REQUIRE_EQUAL(igraph_vector_min(&real_weights), igraph_vector_long_min(&weights));
    igraph_shortest_paths_dijkstra(&graph, &correct_answer, igraph_vss_1(min_ind),
                                   igraph_vss_1(max_ind), &real_weights, IGRAPH_OUT);
    BOOST_REQUIRE_EQUAL((closest_target == -1), (MATRIX(correct_answer, 0, 0) == IGRAPH_INFINITY));
    if (closest_target != -1)
        BOOST_CHECK_EQUAL(MATRIX(correct_answer, 0, 0), VECTOR(mindist)[closest_target]);
    //@todo check memleak when valgrind is available for macOS12

    //free memory
    igraph_destroy(&graph);
    igraph_vector_destroy(&real_weights);
    igraph_vector_long_destroy(&weights);
    igraph_vector_long_destroy(&node_excess);
    igraph_vector_destroy(&backtrack);
    igraph_vector_long_destroy(&excess);
    igraph_matrix_destroy(&correct_answer);
}

BOOST_AUTO_TEST_CASE (dijkstra_with_edge_addition) {
    unsigned int seed = time(NULL);
    for (int i = 0; i < 3; i++) {
        //build random clique with edge addition and check Dijkstra at each iteration
        igraph_t clique;
        igraph_integer_t size = 20; //number of vertices in a clique
        igraph_empty(&clique, size, true); //directed graph

        //generate edge sequence
        newEdges new_edges;
        srand(++seed);
        for (int i = 0; i < size; i++) {
            for (int j = i+1; j < size; j++) {
                int ind = i*size + j;
                newEdge new_edge;
                new_edge.exists = true;
                new_edge.weight = rand() % 200;
                new_edge.source_node = i;
                new_edge.target_node = j;
                new_edge.capacity = 1;
                new_edges.push_back(new_edge);
            }
        }
        std::random_shuffle(new_edges.begin(), new_edges.end());

        igraph_integer_t source = 0;
        igraph_integer_t target = size-1;

        igraph_vector_long_t weights;
        igraph_vector_long_init(&weights, 0);
        igraph_vector_long_t node_excess;
        igraph_vector_long_init(&node_excess, size);
        igraph_vector_long_fill(&node_excess, -1);
        igraph_vector_long_set(&node_excess, target, 1);

        newEdges next_edges; //dummy cache with the first nearest neighbors
        next_edges.resize(size);

        mmHeap dheap; //heap used in Dijkstra
        mmHeap gheap; //dummy for dijkstra, NOT used here as actual heap

        //init vectors for zero-size edge list
        igraph_vector_long_t excess;
        igraph_vector_long_init(&excess, 0);

        igraph_vector_long_t mindist; //minimum distance in Dijstra execution
        igraph_vector_long_t potentials;
        igraph_vector_long_init(&potentials, size);
        igraph_vector_long_fill(&potentials, 0);
        igraph_vector_t backtrack; //ids of ancestor in Dijkstra tree
        init_dijsktra(size, &dheap, &mindist, &backtrack, source);

        igraph_integer_t closest_target;

        //while graph is not clique
        for (int i = 0; i < size*(size-1)/2; i++) {
            //add edge and run Dijsktras
            igraph_integer_t old_ecount = igraph_ecount(&clique);
            addNewEdge(&clique, &dheap, new_edges[i], &weights, &excess, &potentials, &mindist);
            BOOST_CHECK_EQUAL(old_ecount, igraph_ecount(&clique)-2);
            igraph_integer_t from, to;
            igraph_edge(&clique, old_ecount, &from, &to);
            BOOST_CHECK_EQUAL(from, new_edges[i].source_node);
            BOOST_CHECK_EQUAL(to, new_edges[i].target_node);
            //run dijkstra
            closest_target = dijkstra(&clique, &dheap, &gheap, next_edges, &mindist, &excess, &node_excess, &potentials, &weights, &backtrack);

            //check with igraph dijkstra
            igraph_t non_neg_graph;
            igraph_empty(&non_neg_graph, igraph_vcount(&clique), true);

            igraph_vector_t real_weights;
            igraph_vector_init(&real_weights, 0);

            for (long i = 0; i < igraph_ecount(&clique); i++) {
                if (VECTOR(weights)[i] >= 0 && VECTOR(excess)[i] > 0) {
                    igraph_integer_t from, to;
                    igraph_edge(&clique, i, &from, &to);
                    igraph_add_edge(&non_neg_graph, from, to);
                    igraph_vector_push_back(&real_weights, VECTOR(weights)[i]);
                }
            }

            igraph_matrix_t correct_answer;
            igraph_matrix_init(&correct_answer, 0, 0);
            BOOST_REQUIRE_EQUAL(igraph_vector_max(&real_weights), igraph_vector_long_max(&weights));
            igraph_shortest_paths_dijkstra(&non_neg_graph, &correct_answer, igraph_vss_1(source),
                                           igraph_vss_1(target), &real_weights, IGRAPH_OUT);
            BOOST_REQUIRE_EQUAL((closest_target == -1), (MATRIX(correct_answer, 0, 0) == IGRAPH_INFINITY));
            if (closest_target != -1) {
                BOOST_REQUIRE_EQUAL(MATRIX(correct_answer, 0, 0), VECTOR(mindist)[closest_target]);
            }
            igraph_vector_destroy(&real_weights);
            igraph_destroy(&non_neg_graph);
        }

        igraph_destroy(&clique);
        igraph_vector_long_destroy(&excess);
        igraph_vector_long_destroy(&mindist);
        igraph_vector_long_destroy(&potentials);
        igraph_vector_destroy(&backtrack);
        igraph_vector_long_destroy(&node_excess);
        igraph_vector_long_destroy(&weights);
    }

}

BOOST_AUTO_TEST_CASE (FlowAndPotentialsTest) {
    igraph_t graph;
    igraph_empty(&graph, 5, true);
    igraph_vector_t edges;
    igraph_real_t edge_array[20] = {0,1,1,2,2,3,3,2,2,1,1,0,1,3,2,4,3,1,4,2};
    igraph_vector_long_t excess;
    long excess_array[10] = {9, 2, 9, 0, 0, 0, 10, 10, 10, 10};
    igraph_vector_long_t node_excess;
    long node_excess_array[5] = {-2, -2, 0, 2, 2};

    igraph_vector_t backtrack;
    igraph_real_t bct_array[5] = {0, 0, 1, 2, 2};

    igraph_vector_init_copy(&edges, edge_array, 20);
    igraph_vector_long_init_copy(&excess, excess_array, 10);
    igraph_vector_long_init_copy(&node_excess, node_excess_array, 5);
    igraph_vector_init_copy(&backtrack, bct_array, 5);
    igraph_add_edges(&graph, &edges, NULL);

    igraph_vector_long_t potentials;
    igraph_vector_long_init(&potentials, 5);
    igraph_vector_long_fill(&potentials, 0);

    augmentFlow(&graph, &backtrack, &excess, &node_excess, &potentials, 3);
    long answer[10] = {7, 0, 7, 2, 2, 2, 10, 10, 10, 10};
    for (int i = 0; i < 10; i++) {
        BOOST_CHECK_EQUAL(answer[i],VECTOR(excess)[i]);
    }

    igraph_vector_long_t mindist;
    long mindist_arr[5] = {0, 3, 6, 9, 10};
    igraph_vector_long_init_copy(&mindist, mindist_arr, 5);
    updatePotentials(&mindist, &potentials, 3);
    long answer_potentials[5] = {9, 6, 3, 0, 0};
    for (int i = 0; i < 5; i++) {
        BOOST_CHECK_EQUAL(answer_potentials[i],VECTOR(potentials)[i]);
    }

    igraph_vector_destroy(&edges);
    igraph_vector_long_destroy(&excess);
    igraph_vector_long_destroy(&node_excess);
    igraph_vector_destroy(&backtrack);
    igraph_vector_long_destroy(&potentials);
    igraph_destroy(&graph);
}


BOOST_AUTO_TEST_CASE (testMatching) {
    // test symetric matching for a set of random graphs using igraph matching
    int counter = 0;
    while(counter++ < 1) {
        long half_size = counter+1;
        RandomEdgeGenerator* egg = new RandomEdgeGenerator(half_size, half_size, half_size, 1);
//        LoadedEdgeGenerator* egg = new LoadedEdgeGenerator("/Users/alvis/PhD/fcla/bin/egg.txt");
        long result_weight;
        std::vector<std::vector<long>> result_matching;
        for (long i = 0; i < half_size*2; i++) {
            std::vector<long> vec;
            result_matching.push_back(vec);
        }
        //init capacities (excess)
        igraph_vector_long_t node_excess;
        igraph_vector_long_init(&node_excess, half_size*2);
        igraph_vector_long_fill(&node_excess, 1);
        for (long i = 0; i< half_size; i++) {
            VECTOR(node_excess)[i] = -1;
        }
        minMatch(&node_excess, egg, &result_weight, result_matching);

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
        for (long i = 0; i < egg->edgeMemory.size(); i++) {
            newEdge e = egg->edgeMemory[i];
            igraph_add_edge(&test_graph, e.source_node, e.target_node);
            igraph_vector_push_back(&weights, e.weight);
        }
        //fill graph with non-added edges until full
        for (long i = 0; i < half_size; i++) {
            newEdge e;
            e = egg->getEdge(i);
            while (e.exists) {
                igraph_add_edge(&test_graph, e.source_node, e.target_node);
                igraph_vector_push_back(&weights, e.weight);
                e = egg->getEdge(i);
            }
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
        BOOST_REQUIRE_EQUAL(- matching_weight + half_size*max_w, result_weight);

        delete egg;
        igraph_vector_destroy(&weights);
        igraph_vector_bool_destroy(&types);
        igraph_destroy(&test_graph);
        igraph_vector_long_destroy(&matching);
        igraph_vector_long_destroy(&node_excess);
    }
}

BOOST_AUTO_TEST_CASE (multicapacityTest) {

    long source_n = 2;
    long target_n = 4;
    long target_capacity = 2;
    long source_capacity = 1;
    long total_size = source_n + target_n;

    RandomEdgeGenerator* egg = new RandomEdgeGenerator(source_n, source_n, target_n, target_capacity);

    long result_weight;
    std::vector<std::vector<long>> result_matching;
    for (long i = 0; i < total_size; i++) {
        std::vector<long> vec;
        result_matching.push_back(vec);
    }
    //init capacities (excess)
    igraph_vector_long_t node_excess;
    igraph_vector_long_init(&node_excess, total_size);
    igraph_vector_long_fill(&node_excess, target_capacity);
    for (long i = 0; i< source_n; i++) {
        VECTOR(node_excess)[i] = -source_capacity;
    }
    minMatch(&node_excess, egg, &result_weight, result_matching);

}