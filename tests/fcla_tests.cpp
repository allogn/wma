//
// Created by Alvis Logins on 13/02/2017.
//

#define BOOST_TEST_MODULE fcla_tests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include "EdgeGenerator.h"
#include "helpers.h"
#include "ExploringEdgeGenerator.h"
#include "Network.h"
#include "FacilityChooser.h"
#include "Logger.h"
#include "exceptions.h"

BOOST_AUTO_TEST_CASE (testExplorator) {
    //generate random graph, calculate all-to-all distances and compare them with ExploringGenerator results.
    igraph_t graph;
    std::vector<long> weights;
    igraph_vector_t x, y;
    long vsize = 100;
    generate_random_geometric_graph(vsize,0.1,&graph,weights,&x,&y);

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
    igraph_empty(&graph, 3, false);
    igraph_vector_t edges;
    igraph_real_t edge_array[6] = {0,1,1,2,0,2};
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

    fcla.match();
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(fcla.get_bi_node_id_by_target_node_id(0)), 1);
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(fcla.get_bi_node_id_by_target_node_id(4)), 1);
    for (int i = 1; i < 4; i++)
        BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(fcla.get_bi_node_id_by_target_node_id(i)), 0);
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(0), 0);
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(1), 0);

    std::vector<int> complete_sources(2, 0);
    BOOST_CHECK(!fcla.findSetCover());
    BOOST_CHECK(fcla.increaseCapacities(complete_sources));
    BOOST_CHECK(!fcla.findSetCover());
    BOOST_CHECK(fcla.increaseCapacities(complete_sources));
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(fcla.get_bi_node_id_by_target_node_id(2)), 0);
    for (int i = 0; i < 5; i++)
        if (i != 2) BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(fcla.get_bi_node_id_by_target_node_id(i)), 1);
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(0), 0);
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(1), 0);

    BOOST_CHECK(!fcla.findSetCover());
    BOOST_CHECK(fcla.increaseCapacities(complete_sources));
    BOOST_CHECK(!fcla.findSetCover());
    BOOST_CHECK(fcla.increaseCapacities(complete_sources));
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(fcla.get_bi_node_id_by_target_node_id(2)), 2);
    for (int i = 0; i < 5; i++)
        if (i != 2) BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(fcla.get_bi_node_id_by_target_node_id(i)), 1);
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(0), 0);
    BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(1), 0);
    BOOST_CHECK(fcla.findSetCover());

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

BOOST_AUTO_TEST_CASE (test3TrippleGreedy) {
    igraph_t graph;
    std::vector<long> edges = {0,1,1,2,3,4,4,2,5,6,6,2};
    std::vector<long> weights = {1,2,3,4,5,6};
    std::vector<long> sources = {0,3,5};
    create_graph(&graph, 7, edges);
    Network net(&graph, weights, sources);
    Logger logger;
    FacilityChooser fcla(net, 2, 2, &logger);
    fcla.greedyMatching = true;
    fcla.locateFacilities();
    fcla.calculateResult();
    BOOST_CHECK_EQUAL(fcla.result.size(), 2);
    BOOST_CHECK((fcla.result[0] == 2) || (fcla.result[1] == 2));
    igraph_destroy(&graph);
}

void compare_edges(std::pair<long, long>& e1, std::pair<long,long>& e2) {
    BOOST_CHECK_EQUAL(e1.first, e2.first);
    BOOST_CHECK_EQUAL(e1.second, e2.second);
}

BOOST_AUTO_TEST_CASE (gridGraphWithTiedPotentialFacilitiesAndMulticapacity) {
    std::vector<long> edges = {0,3,0,1,1,4,1,2,2,5,3,6,3,4,4,7,4,5,5,8,6,7,7,8,9,10,10,11,0,12};
    std::vector<long> weights = {6,1,2,12,13,30,7,20,3,4,11,5,30,40,0};
    std::vector<long> sources = {0,10,12};
    std::vector<long> targets = {7,5,9};
    long graph_size = 13;

    igraph_t graph;
    create_graph(&graph, graph_size, edges);
    Network net(&graph, weights, sources);
    net.set_target_indexes(targets, 1);
    Logger logger;
    FacilityChooser fcla(net, 3, 0, &logger);
    fcla.locateFacilities();

    for (int i = 3; i < 6; i++) {
        BOOST_CHECK_EQUAL(fcla.get_bi_outdegree(i), 1);
    }
    BOOST_CHECK_EQUAL(fcla.get_bi_node_id_by_target_node_id(5), 4);
    BOOST_CHECK_EQUAL(fcla.edges.size(),7); //bug? extra node?

    std::pair<long,long> e1(fcla.get_source_id_by_node_id(0), -6);
    compare_edges(fcla.edges[fcla.get_bi_node_id_by_target_node_id(5)].front(), e1);
    std::pair<long,long> e2(fcla.get_source_id_by_node_id(12), -15);
    compare_edges(fcla.edges[fcla.get_bi_node_id_by_target_node_id(7)].front(), e2);
    std::pair<long,long> e3(fcla.get_source_id_by_node_id(10), -30);
    compare_edges(fcla.edges[fcla.get_bi_node_id_by_target_node_id(9)].front(), e3);

    TargetExploringEdgeGenerator<long, long> bigraph_generator(*fcla.network, targets);
    std::vector<long> excess({-1,-1,-1,1,1,1});
    Matcher<long,long,long> M(&bigraph_generator, excess, &logger);
    M.match();
    BOOST_CHECK_EQUAL(M.edges.size(),7); //one goes for an extra node (excess)
    for (int i = 3; i < 6; i++) {
        BOOST_CHECK_EQUAL(M.get_bi_outdegree(i), 1);
    }
    BOOST_CHECK_EQUAL(M.get_bi_outdegree(6), 0);

    std::pair<long,long> e11(fcla.get_source_id_by_node_id(0), -6);
    compare_edges(M.edges[fcla.get_bi_node_id_by_target_node_id(5)].front(), e11);
    std::pair<long,long> e22(fcla.get_source_id_by_node_id(12), -15);
    compare_edges(M.edges[fcla.get_bi_node_id_by_target_node_id(7)].front(), e22);
    std::pair<long,long> e33(fcla.get_source_id_by_node_id(10), -30);
    compare_edges(M.edges[fcla.get_bi_node_id_by_target_node_id(9)].front(), e33);
    M.calculateResult();
    BOOST_CHECK_EQUAL(M.result_weight, 51);

    double objective = fcla.calculateResult();
    std::vector<long> result = fcla.get_chosen_facility_node_ids();
    BOOST_CHECK_EQUAL(result.size(), 3);
    std::sort(result.begin(), result.end());
    std::vector<long> ans = {5,7,9};
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), ans.begin(), ans.end());
    BOOST_CHECK_EQUAL(objective, 51);
    igraph_destroy(&graph);
}

BOOST_AUTO_TEST_CASE (lonelyCustomerInTwoComponentGraphExceptionExpected) {
    std::vector<long> edges = {0,1,0,2,2,3,1,3};
    std::vector<long> weights(4, 1);
    std::vector<long> sources = {0,4};
    std::vector<long> targets = {1,2};
    std::vector<long> result = {0,1};

    igraph_t graph;
    create_graph(&graph, 5, edges);
    Network net(&graph, weights, sources);
    net.set_target_indexes(targets, 2);
    Logger logger;

    FacilityChooser fcla(net, 2, 1, &logger);
    //BOOST_CHECK_THROW(fcla.check_feasibility(), InfeasibleSourceAssignmentException); facility indexes check is not implemented, only in gurobi
    fcla.result = result;
    fcla.state = FacilityChooser::State::LOCATED;
    BOOST_CHECK_THROW(fcla.calculateResult(), std::logic_error); //don't allow to compute objective on incorrect result

    FacilityChooser fcla2(net, 2, 1, &logger);
    BOOST_CHECK_THROW(fcla2.locateFacilities(), NoMoreCapacitiesToIncrease);
}