'''
Solve Gurobi for equal average capacities and save result to calculate objective
later
'''

from gurobipy import *
from Network import *
import sys
import re
import json
import time
import pickle as pkl
import os.path

# Gurobi docs : http://examples.gurobi.com/facility-location/

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
        experiment_info['error'] = 'not enough facilities'
        return experiment_info

    if number_of_facilities > len(facility_index):
        experiment_info['error'] = 'not enough potential facility places'
        return experiment_info

#    if not nx.is_connected(network.G):
#        experiment_info['error'] = 'graph is not connected'
#        return experiment_info

    m = Model()
    m.setParam('OutputFlag', False)
    # m.setParam('Threads', 0)
    # m.setParam('LogFile', 'gurobi.log')

    # Add variables
    x = {}
    y = {}
    d = {} # Distance matrix (not a variable)

    for j in range(numFacilities):
        x[j] = m.addVar(vtype=GRB.BINARY, name="x%d" % j)


    start_time = time.time()
    calcdist = False
    if (distmatx != "") and (os.path.isfile(distmatx)):
        pkl_f = open(distmatx, "rb")
        d = pkl.load(pkl_f)
        pkl_f.close()
    else:
        calcdist = True
        #p = nx.shortest_path_length(network.G,weight="weight")
    for i in range(numClients):
        for j in range(numFacilities):
            y[(i, j)] = m.addVar(vtype=GRB.BINARY, name="t%d,%d" % (i,j))
            if (calcdist):
                try:
                    d[(i, j)] = nx.shortest_path_length(network.G,source=network.sources[i],target=facility_index[j],weight="weight") #p[network.sources[i]][facility_index[j]]
                except KeyError:
                    d[(i,j)] = float("inf")

    if (calcdist) and (distmatx != ""):
        pkl_f = open(distmatx, "wb")
        pkl.dump(d, pkl_f)
        pkl_f.close()

    end_time = time.time()

    for i in range(numClients):
        for j in range(numFacilities):
            m.addConstr(y[(i, j)] <= x[j])

    # each customer should be assigned once only
    for i in range(numClients):
        m.addConstr(quicksum(y[(i, j)] for j in range(numFacilities)) == 1)

    # each facility should be assigned not more than capacity times
    for j in range(numFacilities):
        m.addConstr(quicksum(y[(i, j)] for i in range(numClients)) <= facility_capacity)

    # state the exact amount of facilities to be placed
    m.addConstr(quicksum(x[j] for j in range(numFacilities)) == number_of_facilities)

    m.setObjective(quicksum(quicksum(d[(i, j)]*y[(i, j)]
                   for i in range(numClients)) for j in range(numFacilities)))
    m.optimize()


    #get x variables
    xget = {}
    for v in m.getVars():
        if re.match( r'x[0-9]+', v.VarName):
            xget[int(v.VarName[1:])] = v.X

    m2 = Model()
    m2.setParam('OutputFlag', False)
    for i in range(numClients):
        for j in range(numFacilities):
            y[(i, j)] = m2.addVar(vtype=GRB.BINARY, name="t%d,%d" % (i,j))

    # only located facilities can be assigned
    for i in range(numClients):
        for j in range(numFacilities):
            m2.addConstr(y[(i, j)] <= xget[j])

    # each customer should be assigned once only
    for i in range(numClients):
        m2.addConstr(quicksum(y[(i, j)] for j in range(numFacilities)) == 1)

    # each facility should be assigned not more than capacity times
    for j in range(numFacilities):
        m2.addConstr(quicksum(y[(i, j)] for i in range(numClients)) <= capacities[facility_index[j]])

    # state the exact amount of facilities to be placed
    m2.addConstr(quicksum(xget[j] for j in range(numFacilities)) == number_of_facilities)

    m2.setObjective(quicksum(quicksum(d[(i, j)]*y[(i, j)]
                   for i in range(numClients)) for j in range(numFacilities)))
    m2.update()
    m2.optimize()


    if m2.status == GRB.Status.OPTIMAL:
        experiment_info['objective'] = int(m2.objVal)
        experiment_info['runtime'] = m.runtime
        experiment_info['sptime'] = end_time - start_time #time for distance matrix calculation
    else:
        experiment_info['error'] = 'problem infeasible'

    return experiment_info

if __name__ == '__main__':
    if len(sys.argv) < 5:
        print("|path, capacity, number of facilities, output_file, (optional) facility_file, (opt) matr file| arguments required.")
        exit()
    # Load graph from FCLA internal format to NetworkX format
    network_file = sys.argv[1]
    facility_capacity = int(sys.argv[2])
    number_of_facilities = int(sys.argv[3])
    if (len(sys.argv) > 5):
        facility_file = sys.argv[5]
    else:
        facility_file = ""
    if (len(sys.argv) > 6):
        matrfile = sys.argv[6]
    else:
        matrfile = ""
    result = run(network_file, facility_capacity, number_of_facilities, facility_file, matrfile)
    outfile = open(sys.argv[4], 'w')
    json.dump(result, outfile)
