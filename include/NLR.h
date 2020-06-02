/*
 * Nearest Location Region baseline
 *
 * 1. Calculate NLR for each customer (the distance to the nearest unoccupied facility)
 * 2. For each node - find NLRs that cover the node
 * 3. Choose the node with the largest NLRs coverage
 *
 * Note that for the first iteration if there is no existing facility, then a full distance matrix will be
 * calculated and sorted nearest neighbour list created. Then, only nearest neighbor list traversal is required,
 * that takes the same time if all nodes are available for facilities.
 */

#ifndef FCLA_NLR_H
#define FCLA_NLR_H

#include <vector>
#include <forward_list>
#include <stdexcept>
#include <map>

#include "Logger.h"
#include "Matcher.h"
#include "TargetExploringEdgeGenerator.h"
#include "Network.h"
#include "helpers.h"
#include "exceptions.h"

class NLR {
public:
    struct {
        bool any_facility_nlr = true; // NLRs are calculated as the distance to any placed facility, ignoring capacitated ones
    } alg_params;

    std::vector<long> located_facility_indexes; // indexes of located facilities in the current class
    std::vector<long> located_facility_target_indexes; // indexes of located facilities in the graph
    std::vector<long> component_of_potential_facility_location;
    std::vector<long> facilities_available_per_component; // number of allowed facilities per component - prevents a bug of infeasible solution for multicomponents
    long required_facilities;
    std::string exp_id;
    Network* network;
    std::vector<long> customer_indexes;
    std::vector<long> facility_indexes;
    std::map<long, long> inverse_facility_index; //opposite to facility_indexes
    std::vector<long> facility_capacities;

    // for next vectors the size and content correspond to facility_indexes,
    // i.e. facility_located[0] is a state of a facility with index facility_indexes[0]
    std::vector<bool> facility_located; // indicate if potential facility location is occupied by placed facility
    std::vector<bool> facility_capacitated; // indicate if potential facility location is capacitated

    std::vector<std::forward_list<long>> nearest_facilities; //facilities per customer (indexes in facility_indexes)
    std::vector<std::vector<long>> nlrs; //inverse index per facility

    EdgeGenerator* edge_generator;
    Logger* logger;

    long objective;

    void reset() {
        //setting variables that should be cleaned for each algorithm run
        this->located_facility_indexes.clear();
        this->located_facility_target_indexes.clear();
        this->facility_located.clear();
        this->facility_located.resize(this->facility_indexes.size(), false);
        this->facility_capacitated.clear();
        this->facility_capacitated.resize(this->facility_indexes.size(), false);
        this->nearest_facilities = std::vector<std::forward_list<long>>(this->customer_indexes.size(), std::forward_list<long>());
        this->edge_generator->reset();
        clearNLRs();
    }

    void buildInverseFacilityIndex() {
        for (long i = 0; i < this->facility_indexes.size(); i++) {
            this->inverse_facility_index[this->facility_indexes[i]] = i;
        }
    }

    inline bool ifFacilityInNLR(long facility_index) {
        if (!this->facility_located[facility_index]) {
            return true;
        }

        if (!this->alg_params.any_facility_nlr && this->facility_capacitated[facility_index]) {
            return true;
        }
        return false; // facility is capacitated and/or located
    }

    void calculateNLR(long customer_index) {
        // add the customer to nlrs of all facilities that are closer than the closest occupied facility
        for(auto const& nearest_facility_index: this->nearest_facilities[customer_index]) {
            if (!ifFacilityInNLR(nearest_facility_index)) return;
            this->nlrs[nearest_facility_index].push_back(customer_index);
        }
        //no available facilities found -> fetch more edges
        this->getMoreEdgesUntilOccupiedFacilityFound(customer_index);
    }

    void getMoreEdgesUntilOccupiedFacilityFound(long customer_index) {
        for (long i = 0; i < this->facility_indexes.size()+1; i++) { // +1 because we want to fetch all edges until no edges left. +1 will lead to edges further than the last facility
            newEdge edge = this->edge_generator->getEdge(customer_index);
            edge.target_node -= this->customer_indexes.size(); //api of edge generator return id of a node in bgraph
            if (!edge.exists) return; // we traversed all graph and all facilities are in NLR
            long facility_index = edge.target_node;
//            long facility_index;
//            assert(this->inverse_facility_index.find(edge.target_node) != this->inverse_facility_index.end());
//            facility_index = this->inverse_facility_index.at(edge.target_node); //throw out_of_range exception if not found
            this->nearest_facilities[customer_index].push_front(facility_index);

            if (!ifFacilityInNLR(facility_index)) return;
            this->nlrs[facility_index].push_back(customer_index);
        }
        throw std::out_of_range("Trying to fetch too many edges from the graph");
    }

    void clearNLRs() {
        this->nlrs = std::vector<std::vector<long>>(this->facility_indexes.size(), std::vector<long>());
    }

    void calculateAllNLRs() {
        clearNLRs();
        for(long i = 0; i < this->customer_indexes.size(); i++) {
            calculateNLR(i);
        }
    }

    void matchFacilities(bool allow_infeasible = true) {
        // Note that matching problem might be infeasible, having too few placed facilities
        std::vector<long> new_excess(this->customer_indexes.size() + this->located_facility_indexes.size(), -1);

        for (long i = this->customer_indexes.size(); i < new_excess.size(); i++) {
            long facility_index = this->located_facility_indexes[i-this->customer_indexes.size()];
            new_excess[i] = this->facility_capacities[facility_index];
        }

        TargetExploringEdgeGenerator<long, long> bigraph_generator(*this->network, this->located_facility_target_indexes);
        Matcher<long,long,long> M(&bigraph_generator, new_excess, this->logger);
        M.network = this->network;
        M.match();

        // 3-level index mapping: facility index in this class -> index in graph -> index in bgraph
        for(long i = 0; i < this->facility_indexes.size(); i++) {
            long potential_facility_index_in_this_class = i;
            if (this->facility_located[potential_facility_index_in_this_class]) {
                long located_facility_index_in_graph = this->facility_indexes[potential_facility_index_in_this_class];
                long located_facility_index_in_bgraph = bigraph_generator.getIndexOfFacilityInBGraph(located_facility_index_in_graph);
                assert(located_facility_index_in_bgraph >= 0);
                this->facility_capacitated[i] = M.ifTargetCapacitated(located_facility_index_in_bgraph);
            }
        }

        if (!allow_infeasible) {
            M.calculateResult(); //for the last iteration matching must be feasible, otherwise FL is infeasible
            this->objective = M.result_weight;
        }
    }

    void placeAllFacilities() {
        for(long i = 0; i < required_facilities; i++) {
//            this->logger->start("matching");
//            matchFacilities();
//            this->logger->finish("matching");
            this->logger->start("calculate nlrs");
            calculateAllNLRs();
            this->logger->finish("calculate nlrs");
            this->logger->start("place facility");
            placeFacility();
            this->logger->finish("place facility");
        }
    }

    inline long getBestFacility() {
        // traverse NLRs and find the most covered one
        long maxIndex;
        long maxCoverage = -1;
        for (long i = 0; i < this->facility_indexes.size(); i++) {
            long coverage = this->nlrs[i].size();
            long fac_component = this->component_of_potential_facility_location[i];
            bool available_in_component = this->facilities_available_per_component[fac_component] > 0;
            if (coverage > maxCoverage && !this->facility_located[i] && available_in_component) {
                maxIndex = i;
                maxCoverage = coverage;
            }
        }
        if (maxCoverage == -1) {
            throw infeasible_solution; //all facilities are occupied, but customers are not yet satisfied
        }
        return maxIndex;
    }

    void placeFacility() {
        long best_facility = getBestFacility();
        this->located_facility_indexes.push_back(best_facility);
        this->located_facility_target_indexes.push_back(this->facility_indexes[best_facility]);
        this->facility_located[best_facility] = true;
        
        long fac_component = this->component_of_potential_facility_location[best_facility];
        this->facilities_available_per_component[fac_component]--;
    }

    void calculateObjective() {
        this->matchFacilities(false);
        logger->add("objective", this->objective);
    }

    void logParams() {
        this->logger->add("ignore capacitity for NLR", this->alg_params.any_facility_nlr ? 1 : 0);
        this->logger->add("number of facilities", this->required_facilities);
        this->logger->add("id", this->exp_id);
        this->logger->add("capacity of facilities", this->facility_capacities[0]); //@todo fix if multicapacity
    }

    void get_facilities_available_per_component(std::vector<long> customers_per_component,
                                                std::vector<long> places_per_component,
                                                std::vector<long> min_places_per_component) {
        long total_facility_count = this->required_facilities;
        this->facilities_available_per_component.resize(customers_per_component.size());
        long total_customers = network->number_of_customers();
        long left_facilities = total_facility_count;

        // distribute minimum amount
        for (long component_id = 0; component_id < customers_per_component.size(); component_id++) {
            long component_customers = customers_per_component[component_id];
            long facility_capacity = min_places_per_component[component_id];
            long minrequiredfacilities = component_customers/facility_capacity;
            if (component_customers % facility_capacity != 0) {
                minrequiredfacilities++;
            }
            if (left_facilities < minrequiredfacilities) {
                throw infeasible_solution;
            }
            this->facilities_available_per_component[component_id] = minrequiredfacilities;
            left_facilities -= minrequiredfacilities;
        }

        std::vector<double> facility_fraction;
        double total = 0;
        for (long component_id = 0; component_id < customers_per_component.size(); component_id++) {
            long component_customers = customers_per_component[component_id];
            double fraction = static_cast<double>(component_customers);// /static_cast<double>(min_places_per_component[component_id]); -- ignore capacities at all
            total += fraction;
            facility_fraction.push_back(fraction);
        }
        for (auto& f : facility_fraction) {
            f /= total;
        }

        //distribute the rest
        for (long component_id = 0; component_id < customers_per_component.size(); component_id++) {
            long facilities = static_cast<long>(facility_fraction[component_id]*static_cast<double>(total_facility_count));
            long minrequiredfacilities = this->facilities_available_per_component[component_id];
            facilities = std::max(facilities, minrequiredfacilities);
            long delta = facilities - minrequiredfacilities;
            if (delta <= left_facilities) {
                this->facilities_available_per_component[component_id] = facilities;
                left_facilities -= delta;
            } else {
                this->facilities_available_per_component[component_id] += left_facilities;
                left_facilities = 0;
                break;
            }
        }
        assert(left_facilities < customers_per_component.size());
        for (long i = 0; i < left_facilities; i++) {
            this->facilities_available_per_component[i]++;
        }
    }
    
    void calculateMaxFacilitiesPerComponent() {
        igraph_integer_t components;
        igraph_vector_t membership;
        igraph_vector_t csize;
        igraph_vector_init(&membership,0);
        igraph_vector_init(&csize,0);
        logger->start("compute components");
        igraph_clusters(&(network->graph), &membership, &csize, &components, IGRAPH_WEAK);
        logger->finish("compute components");
        logger->add("number of components", components);

        std::vector<long> customers_per_component;
        std::vector<long> capacities_per_component;
        std::vector<long> min_capacity_per_component;
        std::vector<std::vector<long>> node_indexes_per_component;
        for (long component_id = 0; component_id < components; component_id++) {
            //calculate customers
            long customers = 0;
            for (long i = 0; i < network->source_indexes.size(); i++) {
                if (VECTOR(membership)[network->source_indexes[i]] == component_id) {
                    customers++;
                }
            }
            customers_per_component.push_back(customers);
            
            //calculate facilities
            this->component_of_potential_facility_location.resize(network->target_indexes.size());
            for (long i = 0; i < network->target_indexes.size(); i++) {
                long places = 0;
                long min_capacity = 100000;
                if (VECTOR(membership)[network->target_indexes[i]] == component_id) {
                    this->component_of_potential_facility_location[i] = component_id;
                    places += this->facility_capacities[i];
                    if (min_capacity > this->facility_capacities[i]) {
                        min_capacity = this->facility_capacities[i];
                    }
                }
                capacities_per_component.push_back(places);
                min_capacity_per_component.push_back(min_capacity);
            }

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

        get_facilities_available_per_component(customers_per_component, capacities_per_component, min_capacity_per_component);
    }

    NLR(Network& network, Logger* logger, long facility_capacity, long required_facilities) {
        //setting variables once per multiple algorithm runs
        this->logger = logger;
        this->network = &network;
        this->required_facilities = required_facilities;
        this->edge_generator = new TargetExploringEdgeGenerator<long, long>(network, network.target_indexes);
        this->facility_indexes = network.target_indexes;
        buildInverseFacilityIndex();
        this->facility_capacities = network.target_capacities;
        if (this->facility_capacities.size() == 0) {
            this->facility_capacities = std::vector<long>(facility_indexes.size(), facility_capacity);
        }
        assert(this->facility_capacities.size() == this->facility_indexes.size());
        this->customer_indexes = network.source_indexes;
        this->exp_id = network.id;
        
        calculateMaxFacilitiesPerComponent();
    }

    ~NLR() {}

    void run() {
        reset();
        this->logParams();
        try {
            this->logger->start("runtime");
            placeAllFacilities();
            this->logger->finish("runtime");
            calculateObjective();
        } catch (std::exception& e) {
            this->logger->add("error", e.what());
        }
    }

    void set_any_facility_nlr(bool val) {
        this->alg_params.any_facility_nlr = val;
    }
};

#endif //FCLA_NLR_H
