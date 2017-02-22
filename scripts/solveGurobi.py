from gurobipy import *
from Network import *
import sys
import re

# Gurobi docs : http://examples.gurobi.com/facility-location/

def run(network_file,facility_capacity,number_of_facilities):
    network = Network(network_file)
    experiment_id = re.match('.*/([0-9]+).gr',network_file).groups(1)[0]
    footprint = str(experiment_id) + " " + str(number_of_facilities) + " " + str(facility_capacity)
    # Problem data
    clients = network.sources
    facilities = range(network.G.number_of_nodes())

    numFacilities = len(facilities)
    numClients = len(clients)

    if number_of_facilities * facility_capacity < numClients:
        print(footprint + " -1 0")
        return

    if not nx.is_connected(network.G):
        print(footprint + " -1 0")
        return

    m = Model()
    m.setParam('OutputFlag', False)
    # m.setParam('LogFile', 'gurobi.log')

    # Add variables
    x = {}
    y = {}
    d = {} # Distance matrix (not a variable)

    for j in range(numFacilities):
        x[j] = m.addVar(vtype=GRB.BINARY, name="x%d" % j)

    p = nx.shortest_path_length(network.G,weight="weight")
    for i in range(numClients):
        for j in range(numFacilities):
            y[(i, j)] = m.addVar(vtype=GRB.BINARY, name="t%d,%d" % (i,j))
            d[(i, j)] = p[network.sources[i]][j]

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
        m.addConstr(quicksum(y[(i, j)] for i in range(numClients)) <= facility_capacity)

    # state the exact amount of facilities to be placed
    for j in range(numFacilities):
        m.addConstr(quicksum(x[j] for j in range(numFacilities)) == number_of_facilities)

    m.setObjective(quicksum(quicksum(d[(i, j)]*y[(i, j)]
                   for i in range(numClients)) for j in range(numFacilities)))

    m.optimize()

    if m.status == GRB.Status.OPTIMAL:
        print(footprint + " " + str(int(m.objVal)) + " " + str(m.runtime))
    else:
        print(experiment_id + " Error")


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("|path, capacity, number of facilities| arguments required.")
        exit()
    # Load graph from FCLA internal format to NetworkX format

    network_file = sys.argv[1]
    facility_capacity = int(sys.argv[2])
    number_of_facilities = int(sys.argv[3])
    run(network_file, facility_capacity, number_of_facilities)