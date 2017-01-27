//
// Created by allogn on 24.01.17.
//

#define BOOST_TEST_MODULE SIA_tests
#include <boost/test/included/unit_test.hpp>
#include "Bipartite.h"
#include "igraph/igraph.h"
#include "SIA.h"

using namespace SIA;

BOOST_AUTO_TEST_CASE (primitives) {
    //generate random graph and test dijsktra
    igraph_t graph;
    igraph_init()

    igraph_vector_long_t capacities;
    igraph_vector_long_init(&capacities);
    igraph_vector_long_fill(&capacities, 1);

    large_int_t vcount = igraph_vcount(bipartite);
    mmHeap dheap; //heap used in Dijkstra
    igraph_vector_long_t mindist; //minimum distance in Dijstra execution
    igraph_vector_long_t potentials;
    igraph_vector_long_t backtrack; //ids of ancestor in Dijkstra tree
    init_dijsktra(vcount, &dheap, &mindist, &potentials, &backtrack, source_id);

    dijkstra(bigraph, &dheap, &mindist, capacities, )
}