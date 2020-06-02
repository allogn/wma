import networkx as nx
import pandas as pd
import numpy as np
import pickle as pkl
import math
from tqdm import *
import json

import sys
import os
from os import path
sys.path.append(os.path.join(os.environ['PHD_ROOT'],'..','fcla','scripts'))
import helpers
from Network import *

storage_path = '/q/storage/alogins/fcla_data/geometric_with_coords/multicap'

for graph_file in os.listdir(storage_path):
    if graph_file[-4:] != '.ntw':
        continue
   
    network = Network(path.join(storage_path, graph_file))
    new_filename = graph_file[:-4] + ".csv"
    outfile = path.join(storage_path, new_filename)

    sources = set(network.sources)
    hour_attr = {}
    for facility_index in network.G.nodes():
        #if facility_index in sources:
        #    continue
        hour_attr[facility_index] = np.random.randint(1,10)
        
    df = pd.Series(hour_attr,dtype=int)
    df.to_csv(outfile,sep=" ")
    print(new_filename + " generated")
