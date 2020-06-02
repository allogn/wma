from gurobipy import *
from Network import *
import networkx as nx
import sys
import re
import json
import time
import pickle as pkl
import os.path
from tqdm import tqdm
import logging
from networkx.algorithms.shortest_paths.weighted import multi_source_dijkstra_path_length

# Gurobi docs : http://examples.com/facility-location/

def run(network_file,facility_capacity,number_of_facilities,facilityfile="",distmatx=""):
    network = Network(network_file)
    unicapacity = (facilityfile == "")
    experiment_id = network.id
    experiment_info = {}
    experiment_info['id'] = experiment_id
    experiment_info['number of facilities'] = number_of_facilities
    experiment_info['facility_capacity'] = facility_capacity
    experiment_info['multicapacity'] = not unicapacity

    # Problem data
    clients = network.sources

    # Load a list of potential facilities
    capacities = {}
    facility_index = []
    if not unicapacity:
        with open(facilityfile,"r") as f:
            for line in f:
                f_id = int(line.split(" ")[0])
                capacities[f_id] = int(line.split(" ")[1])
                facility_index.append(f_id)
        facilities = capacities
    else:
        facilities = range(network.G.number_of_nodes())
        facility_index = list(range(len(facilities)))
        for i in facility_index:
            capacities[i] = facility_capacity

    numFacilities = len(facilities)
    numClients = len(clients)

    if number_of_facilities * facility_capacity < numClients:
        raise Exception('not enough facilities')

    if number_of_facilities > len(facility_index):
        raise Exception('not enough potential facility places')

    for g in nx.connected_components(network.G):
        number_of_positions = sum([capacities[ind] if ind in g else 0 for ind in facility_index])
        number_of_customers = sum([1 if ind in g else 0 for ind in clients])
        if number_of_positions < number_of_customers:
            raise Exception("not enough pot fac loc (%d pos for %d cust) in the component %d\nNodes: %s" % (number_of_positions, number_of_customers, len(g), str(g)))

    m = Model()
    m.setParam('OutputFlag', False)
    # m.setParam('Threads', 0)
    # m.setParam('LogFile', 'log')

    # Add variables
    x = {}
    y = {}
    d = {} # Distance matrix (not a variable)

    for j in range(numFacilities):
        x[j] = m.addVar(vtype=GRB.BINARY, name="x%d" % j)

    start_time = time.time()
    calcdist = False
    if (distmatx != "") and (os.path.isfile(distmatx)):
        logging.info("Distance matrix found")
        pkl_f = open(distmatx, "rb")
        d = pkl.load(pkl_f)
        pkl_f.close()
    else:
        # p = nx.shortest_path_length(network.G,weight="weight")
        for i in range(numClients):
            dijkstra_dist = nx.algorithms.shortest_paths.weighted.single_source_dijkstra_path_length(network.G, network.sources[i])
            for j in range(numFacilities):
                y[(i, j)] = m.addVar(vtype=GRB.BINARY, name="t%d,%d" % (i,j))
                if (i,j) not in d:
                    try:
                        # d[(i, j)] = p[network.sources[i]][facility_index[j]]
                        #d[(i,j)] = nx.shortest_path_length(network.G,source=network.sources[i],target=facility_index[j],weight="weight")
                        d[(i,j)] = dijkstra_dist[facility_index[j]]
                    except KeyError:
                        d[(i,j)] = GRB.INFINITY#float("inf")
                    except nx.exception.NetworkXNoPath:
                        d[(i,j)] = GRB.INFINITY#float("inf")

        if distmatx != "":
            logging.info("Writing distance matrix to %s" % distmatx)
            pkl_f = open(distmatx, "wb")
            pkl.dump(d, pkl_f)
            pkl_f.close()

    # print("Dictionary size " + str(len(d)))

    end_time = time.time()
    m.update()

    # Add constraints

    # only located facilities can be assigned
    for i in range(numClients):
        for j in range(numFacilities):
            m.addConstr(y[(i, j)] <= x[j])

    # each customer should be assigned once only
    for i in range(numClients):
        m.addConstr(quicksum(y[(i, j)] for j in range(numFacilities)) == 1)

    # each facility should be assigned not more than capacity times
    for j in range(numFacilities):
        m.addConstr(quicksum(y[(i, j)] for i in range(numClients)) <= capacities[facility_index[j]])

    # state the exact amount of facilities to be placed
    m.addConstr(quicksum(x[j] for j in range(numFacilities)) == number_of_facilities)

    m.setObjective(quicksum(quicksum(d[(i, j)]*y[(i, j)]
                   for i in range(numClients)) for j in range(numFacilities)))
    m.optimize()

    if m.status == GRB.Status.OPTIMAL:
        experiment_info['objective'] = int(m.objVal)
        experiment_info['runtime'] = m.runtime
        print(experiment_info['objective'], experiment_info['objective'])

        optlocs = ""
        optlocsind = ""
        for v in m.getVars():
    	    if re.match( r'x[0-9]+', v.VarName):
        		if (v.X == 1):
        		    optlocsind += v.VarName[1:] + ","
        		    optlocs += str(facility_index[int(v.VarName[1:])]) + ","
        experiment_info['optloc'] = optlocs
        experiment_info['optlocind'] = optlocsind
        experiment_info['sptime'] = end_time - start_time #time for distance matrix calculation
    else:
        experiment_info['error'] = 'problem infeasible'
        raise Exception("Problem infeasible")

    return experiment_info

if __name__ == "__main__":
    if len(sys.argv) < 5:
        print("|path, capacity, number of facilities, output_file, (optional) facility_file, (opt) matr file| arguments required.")
        exit()
    network_file = sys.argv[1]
    facility_capacity = int(sys.argv[2])
    number_of_facilities = int(sys.argv[3])
    if (len(sys.argv) > 5) and (sys.argv[5] != "None"):
        facility_file = sys.argv[5]
    else:
        facility_file = ""
    if (len(sys.argv) > 6):
        matrfile = sys.argv[6]
    else:
        matrfile = ""
    result = run(network_file, facility_capacity, number_of_facilities, facility_file, matrfile)
    with open(sys.argv[4], 'w') as outfile:
        json.dump(result, outfile)
