//
// Created by Alvis Logins on 13/02/2017.
//

#define BOOST_TEST_MODULE fcla_tests
#include <boost/test/included/unit_test.hpp>
#include <EdgeGenerator.h>
#include "helpers.h"
#include "ExploringEdgeGenerator.h"
#include "FacilityChooser.h"

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

    FacilityChooser fcla(&graph, weights, source_node_index, 1,2,0);
    fcla.locateFacilities();
    BOOST_REQUIRE_EQUAL(fcla.result.size(),1);
    BOOST_REQUIRE_EQUAL(fcla.result[0], 2);
    igraph_destroy(&graph);
}

/*
 * This will not work because of the method used
 */
//BOOST_AUTO_TEST_CASE (testCliqueSingleFacility) {
////    while (true) {
//        long clique_size = 3;
//        //one place should be reserved for a facility
//        for (long customer_amount = 1; customer_amount < clique_size; customer_amount++) {
//            igraph_t graph;
//            igraph_full(&graph, clique_size, false, false);
//            std::vector<long> weights(igraph_ecount(&graph));
//            for (long i = 0; i < weights.size(); i++) {
//                weights[i] = rand();
//            }
//            std::vector<long> source_node_index(customer_amount);
//            std::vector<long> rand_ind(clique_size);
//            for (long i = 0; i < clique_size; i++) rand_ind[i] = i;
//            std::random_shuffle(rand_ind.begin(), rand_ind.end());
//            for (long i = 0; i < source_node_index.size(); i++) {
//                source_node_index[i] = rand_ind[i];
//            }
//
//            FacilityChooser fcla(&graph, weights, source_node_index, 1, clique_size, 0);
//            fcla.locateFacilities();
////            fcla.edge_generator->save("/Users/alvis/PhD/fcla/bin/tmp.txt");
//
//            //find correct answer
//            long find_min = LONG_MAX;l
////            print_graph(&graph, weights, weights);
//            for (long i = 0; i < clique_size; i++) {
//                long cur_sum = 0;
//                for (long j = 0; j < source_node_index.size(); j++) {
//                    //network graph does not contain self-loops
//                    //but facility can be placed at the same node as customer
//                    //then we count the distance as zero, i.e. skip this edge
//                    if (i == source_node_index[j]) continue;
//                    //get weight of edge
//                    igraph_integer_t eid;
//                    igraph_get_eid(&graph, &eid, source_node_index[j], i, false, true);
//                    cur_sum += weights[eid];
//                }
//                find_min = std::min(find_min, cur_sum);
//            }
//            //calculate fcla weight result
//            long weight_result = 0;
//            BOOST_REQUIRE_EQUAL(fcla.result.size(), 1);
//            long fac_ind = fcla.result[0];
//            for (long j = 0; j < source_node_index.size(); j++) {
//                if (fac_ind == source_node_index[j]) continue;
//                //get weight of edge
//                igraph_integer_t eid;
//                igraph_get_eid(&graph, &eid, source_node_index[j], fac_ind, false, true);
//                weight_result += weights[eid];
//            }
//            BOOST_REQUIRE_EQUAL(weight_result, find_min);
//            igraph_destroy(&graph);
//        }
////    }
//}