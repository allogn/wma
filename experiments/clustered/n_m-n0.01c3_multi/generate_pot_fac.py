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

storage_path = '/q/storage/alogins/fcla_data/clustered/n_m-n0.01c3_multi'

filename = ''
for f in os.listdir(storage_path):
    if f[-4:] == '.ntw':
        filename = f
network = Network(path.join(storage_path, filename))
for number_pot_fac in range(4050,10000,1000):
    for repeat in range(1):
        new_filename = str(number_pot_fac) + "_" + str(repeat) + ".csv"
        outfile = path.join(storage_path, "facilities", new_filename)
        
        facilities = np.array([]) 
        fractions = [len(g)/len(network.G) for g in nx.connected_components(network.G)]
        i = 0
        for g in nx.connected_components(network.G):
            customers = len([s for s in network.sources if s in g])
            m = max(customers, int(fractions[i]*number_pot_fac))
            print("cust {}, m {}, len g {}".format(customers, m, len(g)))
            facilities = np.concatenate((facilities, np.random.choice([n for n in g], m, replace=False)))

            assert(customers <= len(g) and m >= customers and m <= len(g))
            i += 1
        if number_pot_fac != len(facilities): 
            print(number_pot_fac, len(facilities))
            facilities = np.concatenate((facilities, np.random.choice([n for n in network.G if n not in facilities], number_pot_fac - len(facilities), replace=False)))
        assert(number_pot_fac == len(facilities))
        assert(len(set(facilities)) == len(facilities))
        capacity = 15

        hour_attr = {}
        for facility_index in facilities:
            hour_attr[int(facility_index)] = capacity
        
        df = pd.Series(hour_attr,dtype=int)
        df.to_csv(outfile,sep=" ")
