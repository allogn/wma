'''
Script for cleaning raw .ntw graph from small components and unclassified
and irrelevant road types (pedestrian, railway etc)

Export pickled networkx graph, labeled with lat, lon and

Sources of input graph ignored

Usage:
cleanGraph.py sourceFile.ntw targetFile.pkl
or
G = cleanGraph(G)
'''
from tqdm import *
import networkx as nx
import numpy as np
import pickle as pkl
import math
import re
import json
import sys
import time
from geopy.geocoders import Nominatim
from helpers import *


def load_graph(fname, bbox):
    G = nx.Graph()
    node_lat = {}
    node_lon = {}
    graph_id = 0

    redundant_node_set = set()

    if bbox != ():
        print("Extracting bbox...")
        with open(fname,'r') as f:
            print("Looking for redundant nodes...")
            t = f.readline().split()
            graph_id = int(t[0])
            nodes = int(t[1])
            edges = int(t[2])

            for i in range(edges):
                f.readline()

            left_nodes = nodes # because some lines may not contain a node
            while left_nodes > 0:
                info = f.readline().split()
                node_id = nodes-left_nodes
                try:
                    lat = float(info[0])
                    lon = float(info[1])
                    if not ifInBox(lon, lat, bbox):
                        redundant_node_set.add(node_id)
                    left_nodes -= 1
                except ValueError:
                    pass #some info of previous node is split in lines - just ignore it
                except IndexError:
                    pass
            print("Filtered %d out of %d nodes" % (len(redundant_node_set), nodes))

    with open(fname,'r') as f:
        t = f.readline().split()
        graph_id = int(t[0])
        nodes = int(t[1])
        print("Nodes: %d" % nodes)
        edges = int(t[2])
        print("Edges: %d" % edges)

        for i in tqdm(range(edges)):
            info = f.readline().split()
            try:
                id1 = int(info[0])
                id2 = int(info[1])

                if id1 in redundant_node_set or id2 in redundant_node_set:
                    continue

                assert(id1 < nodes)
                assert(id2 < nodes)
                edgetype = "undefined"
                if len(info) > 3:
                    edgetype = info[3]
                G.add_edge(id1, id2, weight=info[2], etype=edgetype)
            except:
                print(info)
                exit()

        left_nodes = nodes # because some lines may not contain a node
        print("Nodes")
        while left_nodes > 0:
            info = f.readline().split()
            try:
                lat = float(info[0])
                lon = float(info[1])
            except ValueError:
                continue #some info of previous node is split in lines - just ignore it
            except IndexError:
                continue

            node_id = nodes-left_nodes
            if node_id % 100000 == 0:
                print("%d, graph %d" % (node_id, len(G)))
            left_nodes -= 1
            if node_id in redundant_node_set:
                continue
            node_lat[node_id] = lat
            node_lon[node_id] = lon
            if node_id not in G:
                G.add_node(node_id)

        print("Graph loaded with ID = %s, resulting size %d" % (t[0], len(G)))
    nx.set_node_attributes(G, name='lat', values=node_lat)
    nx.set_node_attributes(G, name='lon', values=node_lon)
    return G, graph_id

def clean_roads(G):
    print("Cleaning roads...")
    to_delete = []
    for edge in tqdm(G.edges()):
        if G[edge[0]][edge[1]]["etype"] in ["unrelated",'cycleway','pedestrian','footway','steps','path','railway']:
            to_delete.append(edge)
    for e in to_delete:
        G.remove_edge(e[0],e[1])

    to_delete = []
    for node in tqdm(G.nodes()):
        if G.degree(node) < 1:
            to_delete.append(node)
    for n in to_delete:
        G.remove_node(n)
    return G

def remove_small_components(G):
    print("Removing small components...")
    graphs = list(nx.connected_component_subgraphs(G))
    sizes = []
    for g in graphs:
        sizes.append(len(g))
    biggest = np.max(np.array(sizes))
    print("Biggest component: %d" % biggest)
    for g in graphs:
        if len(g) == biggest:
            return g

def export_graph(G, graph_id, filename):
    '''
    Sources is a vector with ids of source nodes
    '''
    new_node_map = {}
    i = 0
    for n in G.nodes(data=True):
        new_node_map[n[0]] = i
        i += 1

    with open(filename,'w') as f:
        f.write("%d %d %d %d\n" % (graph_id, len(G), nx.number_of_edges(G), 0)) # 0 sources
        for e in G.edges(data=True):
            f.write("%d %d %s\n" % (new_node_map[e[0]], new_node_map[e[1]], e[2]['weight']))
        #for i in sources:
        #    f.write("%d\n" % i)
        i = 0
        for n in G.nodes(data=True):
            assert(new_node_map[n[0]] == i)
            i += 1
            f.write("%s %s\n" % (n[1]['lat'], n[1]['lon']))

def cleanGraph(G):
    G = clean_roads(G)
    G = remove_small_components(G)
    return G

def printHelpAndExit():
    print("Args: | sourceFile, targetFile, (opt) bbox < a,b,c,d > / <None>, <delta_lon,delta_lat>, (opt) City Name |")
    exit()

def getPoint(lat, lon):
    return (lon, lat) #dont ask why

def getMetric(bbox, lat0, lon0):
    # print(bbox, lat0, lon0)
    point1 = transform_point(getPoint(bbox[0], bbox[2]), lat0, lon0)
    point2 = transform_point(getPoint(bbox[1], bbox[3]), lat0, lon0)
    return (point1[0], point2[0], point1[1], point2[1])

def ifInBox(lon, lat, bbox):
    return lat <= bbox[1] and lat >= bbox[0] and lon <= bbox[3] and lon >= bbox[2]

if __name__ == '__main__':
    if len(sys.argv) < 3:
        printHelpAndExit()
    start_time = time.time()

    bbox = ()
    if len(sys.argv) > 3:
        lat0 = -1
        lon0 = -1
        try:
            t = sys.argv[4].split(",")
            lat0 = float(t[0])
            lon0 = float(t[1])
        except:
            print("You must mention lat and lon of a corner of universe (thrown by osmtontw)")
            printHelpAndExit()
        if sys.argv[3] == "None":
            cityname = " ".join(sys.argv[5:]) #all the rest is city name
            # geocode city
            geolocator = Nominatim()
            location = geolocator.geocode(cityname)
            if location == None:
                print("City not found")
                exit()
            bbox_arr = location.raw['boundingbox']
            print("BBox:",bbox_arr)
        else:
            bbox_arr = sys.argv[3].split(",")
            if len(bbox_arr) < 4:
                printHelpAndExit()
        bbox = (float(bbox_arr[0]), float(bbox_arr[1]), float(bbox_arr[2]), float(bbox_arr[3]))
        # bbox = (float(bbox_arr[2]), float(bbox_arr[3]), float(bbox_arr[0]), float(bbox_arr[1]))

        #double bbox for vegas
        #bbox = (bbox[1]-2*(bbox[1]-bbox[0]), bbox[1], bbox[2], bbox[3])
        # print(bbox)
        # exit()
        # 56.843050,57.091586,23.879566,24.383220 - riga

        bbox = getMetric(bbox, lat0, lon0)
        print("BBox modif:",bbox)

    G, graph_id = load_graph(sys.argv[1], bbox)
    G = clean_roads(G)
    G = remove_small_components(G)
    export_graph(G, graph_id, sys.argv[2])
    print("Processed by %d sec" % (time.time() - start_time))
