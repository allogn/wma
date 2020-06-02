#define BOOST_TEST_MODULE nlr_tests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include "helpers.h"
#include "NLR.h"
#include "exceptions.h"

BOOST_AUTO_TEST_CASE (testLonelyComponents) {
    igraph_t graph;
    std::vector<long> edges = {0,1,1,2,2,3,3,4,5,6,7,8,8,9,9,10,10,11};
    std::vector<long> weights = {1,1,1,1,1,1,1,1,1};
    std::vector<long> sources = {0,2,5,11};
    create_graph(&graph, 12, edges);

    Network net(&graph, weights, sources);
    std::vector<long> facilities = {1,3,6,8,10};
    std::vector<long> capacities = {3,3,1,1,2};
    net.set_target_indexes(facilities, capacities);

    Logger logger;
    NLR solver(net, &logger, 1, 4);
    BOOST_CHECK_EQUAL(solver.facilities_available_per_component[0],2);
    BOOST_CHECK_EQUAL(solver.facilities_available_per_component[1],1);
    BOOST_CHECK_EQUAL(solver.facilities_available_per_component[2],1);
    BOOST_CHECK_EQUAL(solver.facilities_available_per_component.size(),3);

    solver.run();
    BOOST_CHECK_EQUAL(solver.located_facility_indexes.size(), 4);
    igraph_destroy(&graph);
}

//
//BOOST_AUTO_TEST_CASE (testLonelyComponents) {
//    igraph_t graph;
//    std::vector<long> edges = {0,1,1,5,1,3,1,6,2,3,3,4,3,5,4,5,6,7,6,4,6,5,7,8};
//    std::vector<long> weights = {10,10,10,10,};
//    std::vector<long> sources = {0,2,5,11};
//    create_graph(&graph, 12, edges);
//
//    Network net(&graph, weights, sources);
//    std::vector<long> facilities = {1,3,6,8,10};
//    std::vector<long> capacities = {3,3,1,1,2};
//    net.set_target_indexes(facilities, capacities);
//
//    Logger logger;
//    NLR solver(net, &logger, 1, 4);
//    BOOST_CHECK_EQUAL(solver.facilities_available_per_component[0],2);
//    BOOST_CHECK_EQUAL(solver.facilities_available_per_component[1],1);
//    BOOST_CHECK_EQUAL(solver.facilities_available_per_component[2],1);
//    BOOST_CHECK_EQUAL(solver.facilities_available_per_component.size(),3);
//
//    solver.run();
//    BOOST_CHECK_EQUAL(solver.located_facility_indexes.size(), 4);
//    igraph_destroy(&graph);
//}
