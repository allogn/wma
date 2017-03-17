//
// Created by Alvis Logins on 13/02/2017.
//

#define BOOST_TEST_MODULE fcla_tests
#include <boost/test/included/unit_test.hpp>
#include "EdgeGenerator.h"
#include "helpers.h"
#include "ExploringEdgeGenerator.h"
#include "Network.h"
#include "FacilityChooser.h"
#include "Logger.h"

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
            BOOST_REQUIRE_EQUAL(e.weight, MATRIX(res_matx, source_index[e.source_node], e.target_node-source_index.size()));
        }
        cur_ind = (cur_ind + 1) % vsize;
    }

    igraph_vector_destroy(&x);
    igraph_vector_destroy(&y);
    igraph_vector_destroy(&real_weights);
    igraph_destroy(&graph);
}

BOOST_AUTO_TEST_CASE (basicFacilityLocation) {
    igraph_t graph;
    igraph_empty(&graph, 4, false);
    igraph_vector_t edges;
    igraph_real_t edge_array[20] = {0,1,1,2,0,2};
    igraph_vector_init_copy(&edges, edge_array, 6);
    igraph_add_edges(&graph, &edges, NULL);
    igraph_vector_destroy(&edges);
    std::vector<long> weights = {4,2,1};
    std::vector<long> source_node_index = {0,1};

    Network network(&graph, weights, source_node_index);
    Logger logger;

    FacilityChooser fcla(network, 1, 2, &logger);
    fcla.locateFacilities();
    BOOST_REQUIRE_EQUAL(fcla.result.size(),1);
    BOOST_REQUIRE_EQUAL(fcla.result[0], 2);
    igraph_destroy(&graph);
}

BOOST_AUTO_TEST_CASE (testDuple) {
    igraph_t graph;
    std::vector<long> edges = {0,1,1,2,2,3,3,4};
    std::vector<long> weights = {1,1,1,1,1};
    std::vector<long> sources = {0,4};
    create_graph(&graph, 5, edges);
    Network net(&graph, weights, sources);
    Logger logger;
    FacilityChooser fcla(net, 1, 2, &logger);
    fcla.locateFacilities();
    fcla.calculateResult();
    BOOST_CHECK_EQUAL(fcla.result.size(), 1);
    BOOST_CHECK_EQUAL(fcla.result[0], 2);
    igraph_destroy(&graph);
}

BOOST_AUTO_TEST_CASE (testTripple) {
    igraph_t graph;
    std::vector<long> edges = {0,1,2,1,3,1};
    std::vector<long> weights = {1,2,3};
    std::vector<long> sources = {0,2,3};
    create_graph(&graph, 4, edges);
    Network net(&graph, weights, sources);
    Logger logger;
    FacilityChooser fcla(net, 1, 3, &logger);
    fcla.locateFacilities();
    fcla.calculateResult();
    BOOST_CHECK_EQUAL(fcla.result.size(), 1);
    BOOST_CHECK_EQUAL(fcla.result[0], 1);
    igraph_destroy(&graph);
}

BOOST_AUTO_TEST_CASE (test3Tripple) {
    igraph_t graph;
    std::vector<long> edges = {0,1,1,2,3,4,4,2,5,6,6,2};
    std::vector<long> weights = {1,2,3,4,5,6};
    std::vector<long> sources = {0,3,5};
    create_graph(&graph, 7, edges);
    Network net(&graph, weights, sources);
    Logger logger;
    FacilityChooser fcla(net, 2, 2, &logger);
    fcla.locateFacilities();
    fcla.calculateResult();
    BOOST_CHECK_EQUAL(fcla.result.size(), 2);
    BOOST_CHECK((fcla.result[0] == 2) || (fcla.result[1] == 2));
    igraph_destroy(&graph);
}
