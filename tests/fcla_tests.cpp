//
// Created by Alvis Logins on 13/02/2017.
//

#define BOOST_TEST_MODULE fcla_tests
#include <boost/test/included/unit_test.hpp>
#include <EdgeGenerator.h>
#include "helpers.h"
#include "ExploringEdgeGenerator.h"

BOOST_AUTO_TEST_CASE (testExplorator) {
    //generate random graph, calculate all-to-all distances and compare them with ExploringGenerator results.
    igraph_t graph;
    std::vector<long> weights;
    igraph_vector_t x, y;
    long vsize = 100;
    generate_random_geometric_graph(vsize,0.1,&graph,weights,&x,&y);
//    print_graph(&graph, weights, weights);
    igraph_vector_t real_weights;
    igraph_vector_init(&real_weights, igraph_ecount(&graph));
    for (long i = 0 ; i < weights.size(); i++)
        VECTOR(real_weights)[i] = weights[i];

    igraph_vs_t all_nodes;
    igraph_vs_all(&all_nodes);
    igraph_matrix_t res_matx;
    igraph_matrix_init(&res_matx, 0, 0);
    igraph_shortest_paths_bellman_ford(&graph, &res_matx, all_nodes, all_nodes, &real_weights, IGRAPH_ALL);

    //now calculate for each node in round-robin distances
    std::vector<long> source_index(vsize);
    for (long i = 0; i < vsize; i++) {
        source_index[i] = vsize - i - 1;
    }
    ExploringEdgeGenerator<long,long> edgeGenerator(&graph, weights, source_index);
    long last_ind = -1;
    long cur_ind = 0;
    while (last_ind != cur_ind) {
        if (!edgeGenerator.isComplete(cur_ind)) {
            newEdge e = edgeGenerator.getEdge(cur_ind);
            last_ind = cur_ind;
            BOOST_REQUIRE_EQUAL(e.weight, MATRIX(res_matx, source_index[e.source_node], e.target_node));
        }
        cur_ind = (cur_ind + 1) % vsize;
    }

    igraph_vector_destroy(&x);
    igraph_vector_destroy(&y);
    igraph_vector_destroy(&real_weights);
    igraph_destroy(&graph);
}