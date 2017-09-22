#!/usr/local/bin/python
import networkx as nx
import sys
from Network import *
import re
import os
import numpy as np
import json
import time

if len(sys.argv) < 2:
    print("Provide input directory")
    exit()

path = sys.argv[1]
count = 0
for f in os.listdir(path):
    count += 1
    if f[-4:] != ".ntw":
        continue
    if f[:-4] + ".json" in os.listdir(path):
        continue
    network = Network(os.path.join(path, f))

    time_start = time.time()
    try:
	    info = {}
	    info['id'] = network.id
	    info['vcount'] = network.G.number_of_nodes()
	    info['ecount'] = network.G.number_of_edges()
	    degrees = np.array(nx.degree(network.G).values())
	    info['avg_degree'] = np.mean(degrees)
	    info['std_degree'] = np.var(degrees)
	    info['min_degree'] = np.min(degrees)
	    info['max_degree'] = np.max(degrees)
	    info['avg_clustering'] = nx.average_clustering(network.G)
	    weights = np.array(network.G.edges(data='weight'))
	    info['avg_dist'] = np.mean(weights)
	    info['std_dist'] = np.var(weights)
	    info['min_dist'] = np.min(weights)
	    info['max_dist'] = np.max(weights)
	    info['source_num'] = len(network.sources)
	    info['source_avg_clust'] = nx.average_clustering(network.G,network.sources)
	    degrees2 = np.array(nx.degree(network.G,network.sources).values())
	    info['source_avg_degree'] = np.mean(degrees2)
	    # info['betweenness_centrality'] = nx.betweenness_centrality(network.G,weight='weight')

	    #sps = nx.shortest_path_length(network.G,weight="weight")
	 #   sp_lengths = []
	  #  for i in range(len(network.sources)):
	#	for j in range(i+1,len(network.sources)):
         #           sp_lengths.append(nx.shortest_path(network.G,network.sources[i],network.sources[j],"weight"))
		    #sp_lengths.append(sps[network.sources[i]][network.sources[j]])
	    #sp_lengths = np.array(sp_lengths)
	    #if len(sp_lengths) > 0:
	#	info['source_avg_dist'] = np.mean(sp_lengths)
	#	info['source_std_dist'] = np.var(sp_lengths)
	 #   info['network_diam'] = nx.diameter(network.G,e=nx.eccentricity(network.G,sp=sps))
	  #  eccs = []
	   # for s in network.sources:
	#	eccs.append(nx.eccentricity(network.G, v=s, sp=sps))
	 #   info['source_avg_eccentricity'] = np.mean(np.array(eccs))

	    # save result
	    json.dump(info, open(os.path.join(path, network.id + '.json'),'w'))
	    time_finish = time.time()
	    print(" ".join(("File ", f, " (", str(count), "/", str(len(os.listdir(path))),
		    ") processed in ", str(time_finish - time_start), " seconds")))
    except Exception as e:
        print(e)
        print("file has an error, probably not connected")
        print(network.id)
