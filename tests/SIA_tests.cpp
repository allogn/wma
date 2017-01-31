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
    igraph_vector_long_t excess;
    igraph_vector_long_init(&excess, 1);
    igraph_vector_long_fill(&excess, 1);

    mmHeap dheap; //heap used in Dijkstra
    mmHeap gheap; //dummy for dijkstra
    newEdges next_edges; //dummy for Dijkstra, must contain non-existing edges
    next_edges.resize(2);
    for (int i = 0; i < 2; i++) next_edges[i].exists = false;
    igraph_vector_long_t mindist; //minimum distance in Dijstra execution
    igraph_vector_long_t potentials;
    igraph_vector_t backtrack; //ids of ancestor in Dijkstra tree
    init_dijsktra(2, &dheap, &mindist, &potentials, &backtrack, 0);

    igraph_integer_t closest_target;
    closest_target = dijkstra(&graph, &dheap, &gheap, next_edges, &mindist, &excess, &types, &potentials, &long_weight, &backtrack);
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
    igraph_vector_bool_destroy(&types);
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

    igraph_vector_bool_t types;
    igraph_vector_bool_init(&types, size);
    igraph_vector_bool_fill(&types, false);
    igraph_vector_bool_set(&types, max_ind, true);

    mmHeap dheap; //heap used in Dijkstra
    mmHeap gheap; //dummy for dijkstra
    newEdges next_edges; //dummy for Dijkstra, must contain non-existing edges
    next_edges.resize(size);
    for (int i = 0; i < size; i++) next_edges[i].exists = false;
    igraph_vector_long_t mindist; //minimum distance in Dijstra execution
    igraph_vector_long_t potentials;
    igraph_vector_t backtrack; //ids of ancestor in Dijkstra tree
    init_dijsktra(size, &dheap, &mindist, &potentials, &backtrack, min_ind);

    igraph_integer_t closest_target;
    closest_target = dijkstra(&graph, &dheap, &gheap, next_edges, &mindist, &excess, &types, &potentials, &weights, &backtrack);

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
    igraph_vector_bool_destroy(&types);
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
        igraph_vector_bool_t types;
        igraph_vector_bool_init(&types, size);
        igraph_vector_bool_fill(&types, false);
        igraph_vector_bool_set(&types, target, true);

        newEdges next_edges; //dummy cache with the first nearest neighbors
        next_edges.resize(size);

        mmHeap dheap; //heap used in Dijkstra
        mmHeap gheap; //dummy for dijkstra, NOT used here as actual heap

        //init vectors for zero-size edge list
        igraph_vector_long_t excess;
        igraph_vector_long_init(&excess, 0);

        igraph_vector_long_t mindist; //minimum distance in Dijstra execution
        igraph_vector_long_t potentials;
        igraph_vector_t backtrack; //ids of ancestor in Dijkstra tree
        init_dijsktra(size, &dheap, &mindist, &potentials, &backtrack, source);

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
            closest_target = dijkstra(&clique, &dheap, &gheap, next_edges, &mindist, &excess, &types, &potentials, &weights, &backtrack);

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
    }

}