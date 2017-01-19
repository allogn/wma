//
// Created by allogn on 17.01.17.
//

#ifndef FCLA_ROADNETWORK_H
#define FCLA_ROADNETWORK_H

#include "helpers.h"
#include "igraph/igraph.h"
#include "osmpbfreader.h"
#include "Visitor.h"

using namespace CanalTP;

class RoadNetwork {
public:

    igraph_t graph;
    Visitor* v;

    RoadNetwork() {

#ifndef IGR_CATTRIBUTE_ON
#define IGR_CATTRIBUTE_ON
        //turn on error igraph attribute handling globally
        igraph_i_set_attribute_table(&igraph_cattribute_table);
#endif

        igraph_empty(&graph,0,IGRAPH_DIRECTED);
    };
    ~RoadNetwork() {
        igraph_destroy(&graph);
    };

    void load_pbf(std::string path) {
        v = new Visitor();
        read_osm_pbf(path, *v);
        v->set_graph(&graph);
    }

    bool test_testing() {
        return false;
    }
};


#endif //FCLA_ROADNETWORK_H
