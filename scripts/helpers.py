import networkx as nx
import math as math
import subprocess
import sys
import os
import json
sys.path.append(os.path.join(os.environ['PHD_ROOT'],'wma','scripts'))
data_path = os.path.join(os.environ['ALLDATA_PATH'],'wma')
bin_path = os.path.join(os.environ['PHD_ROOT'],'wma','bin')

def transform_point(point, lat0, lon0):
    lat = point[1]
    lon = point[0]
    dlat = lat - lat0;
    dlon = lon - lon0;
    latitudeCircumference = 40075160. * math.cos(lat0 * math.pi/180.0);
    resX = dlon * latitudeCircumference / 360. ;
    resY = dlat * 40008000. / 360. ;
    return [resX, resY]

def transform_coords(polygon):
    '''
    Transforms all points in shapely polygons
    '''
    new_polygon = []
    for point in polygon:
        new_polygon.append(transform_point(point))
    return new_polygon

def export_graph(G, targetFilename, graph_id, sources):
    '''
    Sources is a vector with ids of source nodes
    '''
    new_node_map = {}
    inverse_node_map = []
    i = 0
    for n in G.nodes(data=False):
        new_node_map[n] = i
        inverse_node_map.append(n)
        i += 1

    with open(targetFilename,'w') as f:
        f.write("%d %d %d %d\n" % (graph_id, len(G), nx.number_of_edges(G), len(sources)))
        for e in G.edges(data=True):
            f.write("%d %d %s\n" % (new_node_map[e[0]], new_node_map[e[1]], e[2]['weight']))
        for i in sources:
            f.write("%d\n" % new_node_map[i])
        for i in range(len(inverse_node_map)):
            node = G.node[inverse_node_map[i]]
            f.write("%s %s\n" % (node['lat'], node['lon']))

def load_graph(fname):
    G = nx.Graph()
    node_lat = {}
    node_lon = {}
    graph_id = 0
    with open(fname,'r') as f:
        t = f.readline().split()
        graph_id = int(t[0])
        nodes = int(t[1])
        print("Nodes: %d" % nodes)
        edges = int(t[2])
        print("Edges: %d" % edges)
        for i in range(edges):
            info = f.readline().split()
            id1 = int(info[0])
            id2 = int(info[1])
            assert(id1 < nodes)
            assert(id2 < nodes)
            G.add_edge(id1, id2, weight=info[2], etype=info[3])

        for i in range(nodes):
            info = f.readline().split()
            try:
                node_lat[i] = float(info[0])
                node_lon[i] = float(info[1])
                if i not in G:
                    G.add_node(i)
            except ValueError:
                print("Error in graph node info: ")
                print(info)
                print("Probably you need to remove new line charachter from graph file")

        print("Graph loaded with ID = %s" % t[0])
    nx.set_node_attributes(G, 'lat', node_lat)
    nx.set_node_attributes(G, 'lon', node_lon)
    return G, graph_id

def run_and_wait(exec_str):
    p = subprocess.Popen(exec_str, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    p.wait()
    out, err = p.communicate()
    if len(err) > 0:
        raise Exception(err.decode('UTF-8') + "\nOutput:\n" + out.decode('UTF-8'))
    if len(out) > 0:
        return out.decode('UTF-8')
    return True

def run_and_wait_wma(graph_file, capacity, number_of_facilities, outfile, facility_file = None):
    exec_str = [
        os.path.join(bin_path,'fcla'),
        '-i',graph_file,
        '-c',str(capacity),
		'-n',str(number_of_facilities),
		'-o',outfile
    ]
    if facility_file:
        exec_str += ['-f',facility_file]
    return run_and_wait(exec_str)

def run_and_wait_gurobi(graph_file, capacity, number_of_facilities, outfile, facility_file = None):
    exec_str = [
        'python', os.path.join(os.environ['PHD_ROOT'],'wma','scripts','solveGurobi.py'),
        graph_file,
        str(capacity),
		str(number_of_facilities),
		outfile
    ]
    if facility_file:
        exec_str += [facility_file]
    return run_and_wait(exec_str)

def load_result(filename):
    return json.load(open(filename,'r'))
