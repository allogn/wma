//
// Created by allogn on 17.01.17.
//

#ifndef FCLA_ROADNETWORK_H
#define FCLA_ROADNETWORK_H

#include <math.h>

#include "helpers.h"
#include <igraph/igraph.h>
#include "osmpbfreader.h"
#include "Visitor.h"
#include "Network.h"

#define PI 3.14159265

using namespace CanalTP;

class RoadNetwork {
public:

    igraph_t graph;
    std::vector<long> weights;
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

    //get euclidean coordinates out of
    inline std::pair<double_t , double_t > get_euclidean(double_t lat0, double_t lon0, double_t lat, double_t lon) {
        double_t dlat = lat - lat0;
        double_t dlon = lon - lon0;
        double_t latitudeCircumference = 40075160. * cos(lat0 * PI/180.0);
        double_t resX = dlon * latitudeCircumference / 360. ;
        double_t resY = dlat * 40008000. / 360. ;
        return std::pair<double_t, double_t >(resX, resY);
    };

    //create X and Y properties in the graph out of lat and lon
    void transform_coordinates() {
        igraph_real_t graph_size = igraph_vcount(&graph);
        igraph_vector_t X;
        igraph_vector_t Y;
        igraph_vector_init(&X, graph_size);
        igraph_vector_init(&Y, graph_size);

        //find minimum lat,lon values for references
        igraph_vector_t all_lat;
        igraph_vector_t all_lon;
        igraph_vector_init(&all_lat, graph_size);
        igraph_vector_init(&all_lon, graph_size);
        VANV(&graph, "lat", &all_lat);
        VANV(&graph, "lon", &all_lon);
        double_t min_lat = static_cast<double_t>(igraph_vector_min(&all_lat));
        double_t min_lon = static_cast<double_t>(igraph_vector_min(&all_lon));

        //calculate new coords in parallel, hope for optimization
        for(uint64_t i = 0; i < static_cast<uint64_t >(graph_size); i++) {
            double_t lat = static_cast<double_t>(igraph_vector_e(&all_lat, i));
            double_t lon = static_cast<double_t>(igraph_vector_e(&all_lon, i));
            std::pair<double_t, double_t> new_coords = get_euclidean(min_lat, min_lon, lat, lon);
            igraph_vector_set(&X, i, new_coords.first);
            igraph_vector_set(&Y, i, new_coords.second);
        }
        SETVANV(&graph, "X", &X);
        SETVANV(&graph, "Y", &Y);
    }

    //add weights based on euclidean distances
    void make_weights() {
        if (!igraph_cattribute_has_attr(&graph, IGRAPH_ATTRIBUTE_VERTEX, "X") ||
                    !igraph_cattribute_has_attr(&graph, IGRAPH_ATTRIBUTE_VERTEX, "Y")) {
            throw "Initialize Euclidean coordinates first by transform_coordinates()";
        }
        weights.clear();
        weights.resize(igraph_ecount(&graph));

        for(uint64_t i = 0; i < igraph_ecount(&graph); i++) {
            igraph_integer_t from;
            igraph_integer_t to;
            igraph_edge(&graph, i, &from, &to);

            igraph_real_t x1 = igraph_cattribute_VAN(&graph, "X", from);
            igraph_real_t y1 = igraph_cattribute_VAN(&graph, "Y", from);
            igraph_real_t x2 = igraph_cattribute_VAN(&graph, "X", to);
            igraph_real_t y2 = igraph_cattribute_VAN(&graph, "Y", to);
            igraph_real_t res = sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));

            weights[i] = res;
        }
    }

    void save_network(std::string outdir, long source_num) {
        std::vector<long> weights;
        std::vector<long> indexes;

        std::vector<long> source_index(igraph_vcount(&graph));
        for (long i = 0; i < source_index.size(); i++) {
            source_index[i] = i;
        }
        std::random_shuffle(source_index.begin(),source_index.end());
        source_index.resize(source_num);

        Network net(&this->graph, this->weights, source_index);
        net.save(outdir);
    }
};


#endif //FCLA_ROADNETWORK_H
