import networkx as nx
import os

class Network:

    def __init__(self):
        self.G = nx.Graph()

    def __init__(self, filename):
        self.load(filename)

    def load(self, filename):
        try:
            file = open(filename, 'r')
        except IOError:
            print("Network file does not exist")
            return
        params = file.readline().split()
        self.id = params[0]
        vcount = int(params[1])
        ecount = int(params[2])
        num_sources = int(params[3])

        self.G = nx.Graph()
        H = nx.path_graph(vcount)
        self.G.add_nodes_from(H)

        for i in range(ecount):
            edgeinfo = file.readline().split()
            from_id = int(edgeinfo[0])
            to_id = int(edgeinfo[1])
            edge_weight = int(edgeinfo[2])
            self.G.add_edge(from_id, to_id, weight=edge_weight)

        self.sources = []
        for i in range(num_sources):
            source_id = int(file.readline())
            self.sources.append(source_id)

        file.close()
