//
// Created by allogn on 18.01.17.
//

#ifndef FCLA_VISITOR_H
#define FCLA_VISITOR_H

#include <map>
#include "osmpbfreader.h"

using namespace CanalTP;

//pbf reader member
class Visitor {
public:
    /*
     * Callbacks are called whenever a OSM member in pbf file is considered
     * Properties of the member are passed into a callback as arguments
     */
    igraph_vector_t nodes;
    std::map<uint64_t, long int > node_index;
    igraph_vector_t edges;
    igraph_vector_t edge_ids;
    igraph_vector_t node_lat;
    igraph_vector_t node_lon;

    Visitor() {
        igraph_vector_init(&nodes, 0);
        igraph_vector_init(&edges, 0);
        igraph_vector_init(&edge_ids, 0);
        igraph_vector_init(&node_lat, 0);
        igraph_vector_init(&node_lon, 0);
    }
    ~Visitor() {
        igraph_vector_destroy(&nodes);
        igraph_vector_destroy(&edges);
        igraph_vector_destroy(&edge_ids);
        igraph_vector_destroy(&node_lat);
        igraph_vector_destroy(&node_lon);
    }

    void node_callback(uint64_t osmid, double lon, double lat, const Tags &tags){
        node_index.insert(std::pair<uint64_t, long int >(osmid,igraph_vector_size(&nodes)));
        igraph_vector_push_back(&nodes, osmid);
        igraph_vector_push_back(&node_lat, lat);
        igraph_vector_push_back(&node_lon, lon);
    }
    void way_callback(uint64_t osmid, const Tags &tags, const std::vector<uint64_t> &refs){
        //add edge along the way and add ID of the way for each added edge
        if (refs.size() < 2)
            return;
        uint64_t prev;
        for (std::vector<uint64_t>::const_iterator it = refs.begin(); it != refs.end(); ++it) {
            //add node reference two times per node: ...->1->2->3->... ---> ...->1 1->2 2->3 3->...
            if (it != refs.begin()) {
                igraph_vector_push_back(&edges,prev);
                igraph_vector_push_back(&edge_ids,osmid);
                igraph_vector_push_back(&edges,*it);
            }
            prev = *it;
        }
    }
    void relation_callback(uint64_t osmid, const Tags &tags, const References &refs){
        //ignore relations so far
        return;
    }

    void set_graph(igraph_t* gp) {
        igraph_empty(gp, igraph_vector_size(&nodes), IGRAPH_UNDIRECTED);

        //map edge osmids to internal node ids
        for (uint64_t i = 0; i < igraph_vector_size(&edges); i++) {
            igraph_real_t nodeid = node_index[igraph_vector_e(&edges, i)];
            igraph_vector_set(&edges, i, nodeid);
        }

        igraph_add_edges(gp, &edges, 0);
        igraph_cattribute_VAN_setv(gp, "lat", &node_lat);
        igraph_cattribute_VAN_setv(gp, "lng", &node_lon);
        igraph_cattribute_EAN_setv(gp, "osmid", &edge_ids);
        igraph_cattribute_VAN_setv(gp, "osmid", &nodes);
    }
};

#endif //FCLA_VISITOR_H
