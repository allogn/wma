//
// Created by allogn on 24.01.17.
//

#define BOOST_TEST_MODULE SIA_tests
#include <boost/test/included/unit_test.hpp>
#include <igraph/igraph.h>
#include <iostream>
#include "helpers.h"
#include "Bipartite.h"
#include "SIA.h"

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
    igraph_vector_bool_t types;
    igraph_vector_bool_init(&types, 2);
    igraph_vector_bool_fill(&types, false);
    igraph_vector_bool_set(&types, 1, true);
    igraph_vector_long_t capacities;
    igraph_vector_long_init(&capacities, 1);
    igraph_vector_long_fill(&capacities, 1);

    mmHeap dheap; //heap used in Dijkstra
    igraph_vector_long_t mindist; //minimum distance in Dijstra execution
    igraph_vector_long_t potentials;
    igraph_vector_t backtrack; //ids of ancestor in Dijkstra tree
    init_dijsktra(2, &dheap, &mindist, &potentials, &backtrack, 0);

    igraph_integer_t closest_target;
    closest_target = dijkstra(&graph, &dheap, &mindist, &capacities, &types, &potentials, &long_weight, &backtrack);
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
    igraph_destroy(&graph);
    igraph_vector_destroy(&real_weight);
    igraph_vector_long_destroy(&long_weight);
    igraph_vector_bool_destroy(&types);
    igraph_vector_destroy(&backtrack);
    igraph_vector_long_destroy(&capacities);
    igraph_matrix_destroy(&correct_answer);
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

    igraph_vector_long_t capacities;
    igraph_vector_long_init(&capacities, size);
    igraph_vector_long_fill(&capacities, INF); // allow any number of outgoing edges from target node

    //find path from left to right
    igraph_integer_t min_ind = static_cast<igraph_integer_t>(igraph_vector_which_min(&x));
    igraph_integer_t max_ind = static_cast<igraph_integer_t>(igraph_vector_which_max(&x));

    igraph_vector_bool_t types;
    igraph_vector_bool_init(&types, size);
    igraph_vector_bool_fill(&types, false);
    igraph_vector_bool_set(&types, max_ind, true);

    mmHeap dheap; //heap used in Dijkstra
    igraph_vector_long_t mindist; //minimum distance in Dijstra execution
    igraph_vector_long_t potentials;
    igraph_vector_t backtrack; //ids of ancestor in Dijkstra tree
    init_dijsktra(size, &dheap, &mindist, &potentials, &backtrack, min_ind);

    igraph_integer_t closest_target;
    closest_target = dijkstra(&graph, &dheap, &mindist, &capacities, &types, &potentials, &weights, &backtrack);

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
}