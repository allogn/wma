from gurobipy import *
from Network import *
import sys

if len(sys.argv) != 4:
    print("<path, capacity, number of facilities> arguments required.")
    exit()

# Gurobi docs : http://examples.gurobi.com/facility-location/

# Load graph from FCLA internal format to NetworkX format
network = Network(sys.argv[1])
facility_capacity = int(sys.argv[2])
number_of_facilities = int(sys.argv[3])

# Problem data
clients = network.sources
facilities = range(network.G.number_of_nodes())

numFacilities = len(facilities)
numClients = len(clients)

if number_of_facilities * facility_capacity < numClients:
    print("Not enough facilities")
    exit()

if not nx.is_connected(network.G):
    print("Graph is not connected")
    exit()

m = Model()

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

m.setParam('OutputFlag', False)
m.setParam('LogFile', '../temp/gurobi.log')
m.optimize()
if m.status == GRB.Status.OPTIMAL:
    print(int(m.objVal))
else:
    print("Solution not found")
