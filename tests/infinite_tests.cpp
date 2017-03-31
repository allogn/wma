#define BOOST_TEST_MODULE infinite_tests
#define _DEBUG_ 1
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <algorithm>
#include "EdgeGenerator.h"
#include "helpers.h"
#include "ExploringEdgeGenerator.h"
#include "Network.h"
#include "FacilityChooser.h"
#include "Logger.h"

BOOST_AUTO_TEST_CASE (test_tight_bounds) {
    long long times = 0;
    while(1) {
        cout << "Running exp_id " << (times++) << endl;

        long n = 100;
        long m = 10;
        igraph_t graph;
        igraph_vector_t x;
        igraph_vector_t y;
        std::vector<long> weights;
        generate_random_geometric_graph(100, 0.4, &graph, weights, &x, &y);
        igraph_vector_destroy(&x);
        igraph_vector_destroy(&y);

        //place random sources
        std::vector<long> sources(n);
        for (long i = 0 ; i < n; i++) sources[i] = i;
        std::random_shuffle(sources.begin(), sources.end());
        sources.resize(m);

        Network net(&graph, weights, sources);
        net.save("./","tmp.ntw");
        cout << "saved" << endl;
        Logger logger;
        FacilityChooser fcla(net, m/2, 2, &logger, 1);
        cout << "created" << endl;
        fcla.locateFacilities();
        cout << "located" << endl;
        fcla.calculateResult();
        cout << "Finished in " << logger.float_dict["runtime"][0] << " sec" << endl;

        igraph_destroy(&graph);
    }
}