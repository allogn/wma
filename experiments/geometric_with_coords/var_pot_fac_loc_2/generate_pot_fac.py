import networkx as nx
import pandas as pd
import numpy as np
import pickle as pkl
import math
from tqdm import *
import json
import os
import sys
from os import path
sys.path.append(os.path.join(os.environ['PHD_ROOT'],'..','fcla','scripts'))
import helpers
from Network import *

storage_path = '/q/storage/alogins/fcla_data/geometric_with_coords/var_pot_fac_loc_2'

filename = ''
for f in os.listdir(storage_path):
    if f[-4:] == '.ntw':
        filename = f
network = Network(path.join(storage_path, filename))
for number_pot_fac in range(9900,10002,10):
    new_filename = str(number_pot_fac) + ".csv"
    outfile = path.join(storage_path, "facilities", new_filename)
    facilities = np.random.choice([n for n in network.G.nodes()], number_pot_fac, replace=False)
    capacity = 4

    hour_attr = {}
    for facility_index in facilities:
        hour_attr[facility_index] = capacity
        
    df = pd.Series(hour_attr,dtype=int)
    df.to_csv(outfile,sep=" ")
