'''
 calculate distance matrix for a specific graph

 creates 2 files, one with dist matrix (dictionary), one with single number -
 time for dijkstra calculation
'''
from Network import *
import sys
import re
import json
import time
import pickle as pkl
import os.path
from tqdm import *

# target filename should be without extension, because one file with time also will be created
def getMatrx(sourceFile, targetFilename, facilityfile = ""):
    start_time = time.time()
    network = Network(network_file)
    clients = network.sources
    numClients = len(clients)

    numFacilities = 0
    facility_index = []
    if facilityfile != "":
        with open(facilityfile,"r") as f:
            for line in f:
                f_id = int(line.split(" ")[0])
                facility_index.append(f_id)
                numFacilities += 1
    else:
        numFacilities = network.G.number_of_nodes()
        facility_index = range(numFacilities)

    #p = nx.shortest_path_length(network.G,weight="weight")
    d = {}
    print("Processing clients... (%d in total)" % numClients)
    for i in tqdm(range(numClients)):
        for j in range(numFacilities):
            #p = nx.single_source_dijkstra_path_length(network.G,source=network.sources[i],weight="weight")
            try:
                d[(i,j)] = nx.shortest_path_length(network.G,source=network.sources[i],target=facility_index[j],weight="weight")
            except nx.exception.NetworkXNoPath:
                d[(i,j)] = float("inf")
    end_time = time.time()
    with open(targetFilename + ".txt", "w") as f:
        f.write(str(end_time - start_time))

    pkl_f = open(targetFilename + ".pkl", "wb")
    pkl.dump(d, pkl_f)
    pkl_f.close()

if __name__ == '__main__':
    '''
     if path specified, then pickle file stored at the same directory under .plk extension
    '''
    if len(sys.argv) < 2:
        print("Args: | path or file, (opt) facility_index_file (can be none), (opt) output filename without extension |")
        exit()

    facility_index_file = ""
    outfilename = ""
    if len(sys.argv) > 1 and sys.argv[2] != "None":
        facility_index_file = sys.argv[2]
    if len(sys.argv) > 2:
        outfilename = sys.argv[3]

    # Load graph from FCLA internal format to NetworkX format
    network_file = sys.argv[1]
    if os.path.isfile(network_file):
        # proceed one file
        if network_file[-4:] != ".ntw":
            print("Wrong extension for graph file", network_file[-4:])
            exit()
        if outfilename == "":
            outfilename = network_file[:-4]
        getMatrx(network_file, outfilename, facility_index_file)
    else:
        files_to_process = []
        for f in os.listdir(network_file):
            if f[-4:] == ".ntw":
                files_to_process.append(f)
        for f in tqdm.tqdm(files_to_process):
            getMatrx(f, f[:-4], facility_index_file)
