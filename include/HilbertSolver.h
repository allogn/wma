//
// Created by Alvis Logins on 31/01/2018.
//

#ifndef FCLA_HILBERTSOLVER_H
#define FCLA_HILBERTSOLVER_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include "helpers.h"
#include "Network.h"
#include "FacilityChooser.h"
#include "igraph/igraph.h"
#include "Logger.h"
#include "Hilbert.h"
#include "TargetExploringEdgeGenerator.h"
#include "exceptions.h"

class HilbertSolver {
public:

    Logger* logger;
    Network* network;
    long facility_number_to_locate;
    long facility_capacity;

    HilbertSolver(Network* net, Logger* logger) {
        this->network = net;
        this->logger = logger;
    }

    ~HilbertSolver() {

    }

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
            return hilbert_ieee_cmp(2,d1,d2) >= 0;
        }
    } hilbert_comparator;

    inline double get_dist(Coords& c1, Coords& c2)
    {
        return sqrt(pow(c1.first-c2.first,2) + pow(c1.second-c2.second,2));
    }

    std::vector<long> equally_splitting_indexes(long vec_size, long splits) {
        /*
         * Returns a vector of indexes for splitting vectr of size vec_size to <splits> parts equally.
         * Resulting vector always starts with 0 and ends with vec_size.
         *
         * @return vector of size splits+1
         */
        std::vector<long> result(splits+1, vec_size/splits);
        result[0] = 0;
        for (long i = 1; i < vec_size % splits + 1; i++) {
            result[i]++;
        }
        for (long i = 1; i < splits + 1; i++) {
            result[i] = result[i-1] + result[i];
        }
        assert(result[splits] == vec_size);
        return result;
    }

    std::vector<long> get_facility_node_indexes_in_component(std::vector<Customer>& customers, std::vector<long> potential_facility_node_indexes, long facility_count) {
        /*
         * Return locations of new facilities in the connected component of the graph.
         *
         * Splits customers into k buckets, where k = number of facilities
         * Calculate geocenter for each bucket and return closest node to the geocenter
         *
         * component_node_indexes is copied since modified inside the function
         */

        std::vector<long> facility_locations;
        if (customers.size() == 0 || facility_count == 0) return facility_locations;

        std::sort(customers.begin(), customers.end(), hilbert_comparator);

        std::vector<long> splitting_indexes = equally_splitting_indexes(customers.size(), facility_count);
        for (long i = 0; i < facility_count; i++) {
            if (splitting_indexes[i+1] == splitting_indexes[i]) continue;
            Coords center = geocenter(customers, splitting_indexes[i], splitting_indexes[i+1]);
            if (potential_facility_node_indexes.size() == 0) {
                throw std::string("Not enough potential facility locations");
            }
            long node_index = return_and_remove_closest_node_index(potential_facility_node_indexes, center); //side effect
            facility_locations.push_back(node_index);
        }
        return facility_locations;
    }

    long return_and_remove_closest_node_index(std::vector<long>& node_indexes, Coords center) {
        /*
         * Find best candidate by linear search.
         */
        long best_index = -1;
        double best_dist = INFINITY;
        for (long i = 0; i < node_indexes.size(); i++) {
            double dist = get_dist(center, network->coords[node_indexes[i]]);
            if (dist < best_dist) {
                best_dist = dist;
                best_index = i;
            }
        }
        long best_node_index = node_indexes[best_index];
        node_indexes.erase(node_indexes.begin() + best_index); // prevent the node from being selected again
        return best_node_index;
    }

    Coords geocenter(std::vector<Customer>& customers, long start_index, long end_index)
    {
        assert(end_index > start_index);
        double mean_x = 0;
        double mean_y = 0;
        for (long i = start_index; i < end_index; i++) {
            mean_x += customers[i].coords.first;
            mean_y += customers[i].coords.second;
        }
        mean_x /= end_index-start_index;
        mean_y /= end_index-start_index;
        return std::make_pair(mean_x,mean_y);
    }

    std::vector<long> get_facility_count_per_component(std::vector<std::vector<Customer>> customers_per_component, long total_facility_count) {
        std::vector<long> facility_count(customers_per_component.size());
        long total_customers = network->number_of_customers();
        long left_facilities = total_facility_count;
        // distribute minimum amount
        for (long component_id = 0; component_id < customers_per_component.size(); component_id++) {
            long component_customers = customers_per_component[component_id].size();
            long minrequiredfacilities = component_customers/facility_capacity;
            if (component_customers % facility_capacity != 0) {
                minrequiredfacilities++;
            }
            if (left_facilities < minrequiredfacilities) {
                throw infeasible_solution;
            }
            facility_count[component_id] = minrequiredfacilities;
            left_facilities -= minrequiredfacilities;
        }
        //distribute the rest
        for (long component_id = 0; component_id < customers_per_component.size(); component_id++) {
            long component_customers = customers_per_component[component_id].size();
            double facility_fraction = static_cast<double>(component_customers)/static_cast<double>(total_customers);
            long facilities = static_cast<long>(facility_fraction*static_cast<double>(total_facility_count));
            long minrequiredfacilities = facility_count[component_id];
            facilities = std::max(facilities, minrequiredfacilities);
            long delta = facilities - minrequiredfacilities;
            if (delta <= left_facilities) {
                facility_count[component_id] = facilities;
                left_facilities -= delta;
            } else {
                facility_count[component_id] += left_facilities;
                left_facilities = 0;
                break;
            }
        }
        assert(left_facilities < customers_per_component.size());
        for (long i = 0; i < left_facilities; i++) {
            facility_count[i]++;
        }
        return facility_count;
    }

    bool node_is_potential_facility(long node_index) {
        return std::find(this->network->target_indexes.begin(),
                         this->network->target_indexes.end(),
                         node_index) != this->network->target_indexes.end();
    }

    std::vector<long> get_facility_node_indexes(long facility_number_to_locate)
    {
        igraph_integer_t components;
        igraph_vector_t membership;
        igraph_vector_t csize;
        igraph_vector_init(&membership,0);
        igraph_vector_init(&csize,0);
        logger->start("compute components");
        igraph_clusters(&(network->graph), &membership, &csize, &components, IGRAPH_WEAK);
        logger->finish("compute components");
        logger->add("number of components", components);

        std::vector<std::vector<Customer>> customers_per_component;
        std::vector<std::vector<long>> node_indexes_per_component;
        for (long component_id = 0; component_id < components; component_id++) {
            std::vector<Customer> customers;
            for (long i = 0; i < network->source_indexes.size(); i++) {
                if (VECTOR(membership)[network->source_indexes[i]] == component_id) {
                    Customer new_customer;
                    new_customer.coords.first = network->coords[network->source_indexes[i]].first;
                    new_customer.coords.second = network->coords[network->source_indexes[i]].second;
                    new_customer.index = network->source_indexes[i];
                    customers.push_back(new_customer);
                }
            }
            customers_per_component.push_back(customers);

            std::vector<long> component_node_indexes;
            for (long i = 0; i < network->graph_size(); i++) {
                if (VECTOR(membership)[i] == component_id) {
                    component_node_indexes.push_back(i);
                }
            }
            node_indexes_per_component.push_back(component_node_indexes);
        }
        igraph_vector_destroy(&membership);
        igraph_vector_destroy(&csize);

        // run solver per each component independently
        std::vector<long> result;
        std::vector<long> facility_count_per_component = get_facility_count_per_component(customers_per_component, facility_number_to_locate);
        for (long i = 0; i < components; i++) {

            std::vector<long> potential_facility_locations;
            for (long j = 0; j < node_indexes_per_component[i].size(); j++) {
                if (this->node_is_potential_facility(node_indexes_per_component[i][j])) {
                    potential_facility_locations.push_back(node_indexes_per_component[i][j]);
                }
            }

            std::vector<long> next_set = get_facility_node_indexes_in_component(customers_per_component[i], potential_facility_locations, facility_count_per_component[i]);
            result.insert(result.end(), next_set.begin(), next_set.end());
        }

        assert(result.size() == facility_number_to_locate);
        return result;
    }

    long calculate_objective(std::vector<long> facility_node_indexes) {
        /*
         * calculate matching for result vector
         */
        std::vector<long> new_excess(network->source_indexes.size() + facility_node_indexes.size(),facility_capacity);
        for (long i = 0; i < network->source_indexes.size(); i++) {
            new_excess[i] = -1;
        }
        for (long i = network->source_indexes.size(); i < new_excess.size(); i++) {
            if (network->target_capacities.size() == 0) {
                new_excess[i] = this->facility_capacity;
            } else {
                new_excess[i] = network->target_capacities[i-network->source_indexes.size()];
            }
        }

        std::vector<long> only_target_facility_node_indexes;
        std::set<long> existing_facilities;
        for (auto i = facility_node_indexes.begin(); i != facility_node_indexes.end(); i++) {
            existing_facilities.insert(*i);
        }
        for (long i = 0; i < this->network->target_indexes.size(); i++) {
            long el = this->network->target_indexes[i];
            if (existing_facilities.find(el) != existing_facilities.end()) {
                only_target_facility_node_indexes.push_back(i);
            }
        }

        TargetExploringEdgeGenerator<long,long> edge_generator(*network, only_target_facility_node_indexes);
        Matcher<long,long,long> M(&edge_generator, new_excess, logger);
        M.match();
        M.calculateResult();
        return M.result_weight;
    }

    void run(long facility_number_to_locate, long facility_capacity) {
        this->facility_capacity = facility_capacity;
        this->facility_number_to_locate = facility_number_to_locate;
        logger->add("id", network->id);
        logger->add("number of facilities", facility_number_to_locate);
        logger->add("capacity of facilities", facility_capacity);

        try {

            logger->start("runtime");
            std::vector<long> facility_node_indexes = get_facility_node_indexes(facility_number_to_locate);
//            std::cout << "selected" << std::endl;;
//            for (auto i = facility_node_indexes.begin(); i != facility_node_indexes.end(); i++)
//                std::cout << *i << std::endl;
            logger->finish("runtime");
            long objective = calculate_objective(facility_node_indexes);
            logger->add("objective", objective);
        } catch (exception& e) {
            logger->add("error", e.what());
        }
    }

    void save_log(std::string filename) {
        logger->save(filename);
    }
};

#endif //FCLA_HILBERTSOLVER_H
