/*
 * Facility matching hilbert solver file
 */

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <boost/program_options.hpp>

#include "helpers.h"
#include "Network.h"
#include "FacilityChooser.h"
#include "igraph/igraph.h"
#include "Logger.h"
#include "Hilbert.h"
#include "TargetExploringEdgeGenerator.h"

using namespace std;
namespace po = boost::program_options;

struct Customer {
    Coords coords;
    long index;
};

struct ComparatorHilbertEntry
{
    bool operator()(const Customer& r1, const Customer& r2)
    {
        double d1[2],d2[2];
        d1[0] = r1.coords.first;
        d1[1] = r1.coords.second;
        d2[0] = r2.coords.first;
        d2[1] = r2.coords.second;
        if (hilbert_ieee_cmp(2,d1,d2)>=0)  return true;
        else return false;
    }
} hilbert_comparator;

inline double get_dist(Coords& c1, Coords& c2)
{
    return sqrt(pow(c1.first-c2.first,2) + pow(c1.second-c2.second,2));
}

long get_closest(std::vector<bool>& used, Network& network, igraph_vector_t* membership, long cluster_id, Coords& center) {
    long best_index = -1;
    double best_dist = INFINITY;
    for (long i = 0; i < network.coords.size(); i++) {
        if (VECTOR(*membership)[i] == cluster_id && !used[i]) {
            double dist = get_dist(center, network.coords[i]);
            if (dist < best_dist) {
                best_dist = dist;
                best_index = i;
            }
        }
    }
    assert(best_index > -1);
    used[best_index] = true;
    return best_index;
}

Coords calculateCenter(std::vector<Customer>& customers, long start_index, long end_index)
{
    double mean_x = 0;
    double mean_y = 0;
    for (long i = start_index; i < end_index; i++) {
        mean_x += customers[i].coords.first;
        mean_y += customers[i].coords.second;
    }
    mean_x /= end_index-start_index;
    mean_y /= end_index-start_index;
    return std::make_pair(mean_x,mean_y);
};

int main(int argc, const char** argv) {
    string filename;
    long facilities_to_locate;
    long facility_capacity;
    string out_filename;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("input,i", po::value<string>(&filename)->required(), "Input file, a network")
            ("facilities,n", po::value<long>(&facilities_to_locate)->required(), "Facilities to locate")
            ("faccap,c", po::value<long>(&facility_capacity)->default_value(1), "Capacity of facilities")
            ("output,o", po::value<string>(&out_filename)->required(), "Output file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    po::notify(vm);

    Logger logger;
    logger.start("total time");
    Network net(filename);

    //sort by hilbert, cluster by customers equally and compute a node that is the closest to the center of a cluster
    logger.start("runtime");

    std::vector<long> result;
    std::vector<bool> used(igraph_vcount(&net.graph),false);
    igraph_integer_t components;
    igraph_vector_t membership;
    igraph_vector_t csize;
    igraph_vector_init(&membership,0);
    igraph_vector_init(&csize,0);
    igraph_clusters(&(net.graph), &membership, &csize, &components, IGRAPH_WEAK);
    logger.add("number of components", components);
    logger.start("calculate customers");
    std::vector<std::vector<Customer>> customers_per_component;
    std::vector<long> facility_per_component(components,0);
    long left_facilities = facilities_to_locate;
    for (long component_id = 0; component_id < components; component_id++) {
         std::vector<Customer> v;
         for (long i = 0; i < net.source_indexes.size(); i++) {
            if (VECTOR(membership)[net.source_indexes[i]] == component_id) {
                Customer new_customer;
                new_customer.coords.first = net.coords[net.source_indexes[i]].first;
                new_customer.coords.second = net.coords[net.source_indexes[i]].second;
                new_customer.index = i;
                v.push_back(new_customer);
            }
        }
        std::sort(v.begin(),v.end(),hilbert_comparator);
        customers_per_component.push_back(v);
        facility_per_component[component_id] += (long)ceil((double)v.size() / (double)facility_capacity); 
        left_facilities -= facility_per_component[component_id];
    }
    logger.finish("calculate components");
    long extra = left_facilities;
    for (long component_id = 0; component_id < components; component_id++) {
        long proportion = (long)ceil((double)customers_per_component[component_id].size() * (double)extra / (double)net.source_indexes.size());
        proportion = std::min(proportion, left_facilities);
        left_facilities -= proportion;
        long facilities_per_cluster = proportion + facility_per_component[component_id];
        if (facilities_per_cluster > 0) {
            std::vector<Customer> customers = customers_per_component[component_id];
            long step = (long)floor(customers.size() / facilities_per_cluster);
            for (long i = 0; i < facilities_per_cluster; i++) {
                Coords center = calculateCenter(customers,i*step, (i+1)*step);
                long node_id = get_closest(used,net, &membership, component_id, center);
                result.push_back(node_id);
            }
        }
    }
    assert(result.size() == facilities_to_locate);
    igraph_vector_destroy(&membership);
    logger.finish("runtime");

    /*
     * calculate matching for result vector
     */
    std::vector<long> new_excess(net.source_indexes.size() + result.size(),facility_capacity);
    for (long i = 0; i < net.source_indexes.size(); i++) {
        new_excess[i] = -1;
    }
    TargetExploringEdgeGenerator<long,long> edge_generator(net, result);
    Matcher<long,long,long> M(&edge_generator, new_excess, &logger);
    M.run();
    M.calculateResult();

    logger.add("objective", M.result_weight);
    logger.add("id", net.id);
    logger.add("number of facilities", facilities_to_locate);
    logger.add("capacity of facilities", facility_capacity);

    logger.finish("total time");
    logger.save(out_filename);
    return 0;
}
