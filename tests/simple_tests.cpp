//
// Created by allogn on 17.01.17.
//

// examples: http://www.ibm.com/developerworks/aix/library/au-ctools1_boost/

#define BOOST_TEST_MODULE simple_tests
#include <boost/test/included/unit_test.hpp>
#include "RoadNetwork.h"
#include "Bipartite.h"
#include "TargetEdgeGenerator.h"
#include "Matcher.h"
#include "Network.h"
#include "ExploringEdgeGenerator.h"

BOOST_AUTO_TEST_CASE (example_test)
{
    RoadNetwork r;
    BOOST_CHECK(!r.test_testing());
}

bool check_igraph_vector(igraph_vector_t r, std::vector<uint64_t>& correct) {
    for (int i = 0; i < correct.size(); i++ ) {
        if (correct[i] != igraph_vector_e(&r, i)) return false;
    }
    return true;
}

BOOST_AUTO_TEST_CASE (visitor)
{
    Visitor* v = new Visitor();
    Tags tags;
    BOOST_CHECK(igraph_vector_size(&v->nodes) == 0);
    for (int i = 0; i < 100; i++) {
        v->node_callback(i, (float)rand()/(float)RAND_MAX-0.5, (float)rand()/(float)RAND_MAX-0.5, tags);
    }
    BOOST_CHECK(igraph_vector_size(&v->nodes) == 100);
    BOOST_CHECK(igraph_vector_size(&v->node_lat) == igraph_vector_size(&v->nodes) && igraph_vector_size(&v->node_lon) == igraph_vector_size(&v->nodes));

    uint64_t sum = 0;
    std::vector<uint64_t> way;
    for (uint64_t i = 0; i < 100; i++) {
        way.resize(i,i);
        v->way_callback(i, tags, way);
        if (i >= 2) sum += i-1;
    }
    BOOST_CHECK(igraph_vector_size(&v->edge_ids) == igraph_vector_size(&v->edges)/2);
    BOOST_CHECK(igraph_vector_size(&v->edge_ids) == sum);
    delete v;

    v = new Visitor();
    std::vector<uint64_t> w({1,2,3});
    std::vector<uint64_t> w2({1,2,2,3});
    v->way_callback(0,tags,w);
    for (int i = 0; i < 4; i++) {
        BOOST_CHECK(igraph_vector_e(&v->edges,i) == w2[i]);
    }
    delete v;

    //test graph structure
    v = new Visitor();
    igraph_t g;
    v->set_graph(&g);
    BOOST_CHECK(igraph_vcount(&g) == 0);
    BOOST_CHECK(igraph_ecount(&g) == 0);

    v->node_callback(0, 0.5, 0.5, tags);
    v->set_graph(&g);
    BOOST_CHECK(igraph_vcount(&g) == 1);
    BOOST_CHECK(igraph_ecount(&g) == 0);

    //add edges
    std::vector<uint64_t> more_nodes({1,2,3,4,5});
    for (int i = 0; i < more_nodes.size(); i++) {
        v->node_callback(i+1, (double)i/100., (double)i/100., tags);
    }
    v->set_graph(&g);
    BOOST_CHECK(igraph_vcount(&g) == 6);
    std::vector<uint64_t> path1({1,2,3});
    std::vector<uint64_t> path2({4,5});
    std::vector<uint64_t> path3({2,5});
    v->way_callback(0, tags, path1);
    v->way_callback(1, tags, path2);
    v->way_callback(2, tags, path3);
    BOOST_CHECK(igraph_vector_size(&v->edges) == 8);
    v->set_graph(&g);

    //checking degrees
    igraph_vector_t res;
    igraph_vector_init(&res, 0);
    igraph_vs_t vids;
    igraph_vs_all(&vids);
    //calculate degrees of all vertices and put into res vector
    igraph_degree(&g, &res, vids, IGRAPH_ALL, false);
    std::vector<uint64_t> answer({0,1,3,1,1,2});
    BOOST_CHECK(check_igraph_vector(res, answer));
    delete v;

}

BOOST_AUTO_TEST_CASE (distanceTest) {
    RoadNetwork r;
    Visitor v;
    Tags tags;
    //check distance between aalborg and klarup
    //https://www.distancecalculator.net/from-aalborg-to-copenhagen
    v.node_callback(0, 57.048, 9.9187, tags); //Aalborg
    v.node_callback(1, 57.011564, 10.053957, tags); //Klarup
    std::vector<uint64_t > edge({0,1});
    v.way_callback(0, tags, edge);
    v.set_graph(&r.graph);
    r.transform_coordinates();
    BOOST_CHECK(VAN(&r.graph,"X",1) == 0);
    BOOST_CHECK(VAN(&r.graph,"Y",0) == 0);
    r.make_weights();
    igraph_real_t dist = EAN(&r.graph, "weight", 0);
    BOOST_CHECK_CLOSE_FRACTION(dist, 9025, 1);
}

BOOST_AUTO_TEST_CASE (bipartiteGraphMatching) {
    Bipartite bi;
    long bignumber = 10;
    bi.generate_full_bipartite(bignumber,bignumber);
    bi.generate_random_weights();
    BOOST_CHECK(igraph_ecount(&bi.bigraph) == bignumber*bignumber);
    for (uint64_t i = 0; i < bignumber*(bignumber-1)/2; i++) {
        igraph_integer_t from;
        igraph_integer_t to;
        igraph_real_t weight = EAN(&bi.bigraph, "weight", i);
        igraph_edge(&bi.bigraph, i, &from, &to);
        BOOST_CHECK((weight >= 0) && (weight <= 1));
        BOOST_CHECK(VECTOR(bi.types)[from] != VECTOR(bi.types)[to]);
    }
    igraph_real_t result_weight;
    igraph_vector_long_t result_matching;
    igraph_vector_long_init(&result_matching, 0);
    bi.igraph_max_matching(&result_weight, &result_matching); //check for runtime errors
}

BOOST_AUTO_TEST_CASE (tergetgenerator) {
    igraph_t graph;
    igraph_empty(&graph, 4, false);
    igraph_vector_t edges;
    igraph_real_t edge_array[6] = {0,1,0,2,2,3};
    igraph_vector_init_copy(&edges, edge_array, 6);
    igraph_add_edges(&graph, &edges, NULL);
    igraph_vector_destroy(&edges);
    std::vector<long> weights = {1,2,3};
    std::vector<long> source_node_index = {0};  //@todo source node index must be consistent with node excess for matchers

    Network network(&graph, weights, source_node_index);
    ExploringEdgeGenerator<long, long> old_generator(network);

    std::vector<long> node_excess({-1,1,1,1,1});
    Logger logger;
    Matcher<long,long,long> M(&old_generator, node_excess, &logger);
    M.run();

    std::vector<long> targets({3});
    std::vector<long> node_excess2({-1,1});
    TargetEdgeGenerator generator(&M, targets);
    Matcher<long,long,long> M2(&generator, node_excess2, &logger);
    M2.run();
    M2.calculateResult();
    BOOST_CHECK_EQUAL(M2.result_weight, 5);
}