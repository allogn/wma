import random
import sys
import numpy as np

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print("Usage: python putRandomCustomers.py infile number_of_customers new_file_id_delta")
        exit()
    network_file = sys.argv[1]
    number_of_customers = int(sys.argv[2])
    new_file_id_delta = int(sys.argv[3])

infile = open(network_file, "r")

params = infile.readline().split()
netid = int(params[0])
vcount = int(params[1])
ecount = int(params[2])
old_num_sources = int(params[3])

new_id = str(netid+new_file_id_delta)
outfile = open(new_id + ".ntw", "w")

outfile.write(new_id + " " + params[1] + " " + params[2] + " " + str(number_of_customers) + "\n")

all_nodes = set()
for i in range(ecount):
    edgeinfo = infile.readline()
    from_id = int(edgeinfo.split()[0])
    to_id = int(edgeinfo.split()[1])
    all_nodes.add(from_id)
    all_nodes.add(to_id)
    outfile.write(edgeinfo)

for i in range(old_num_sources):
    source_id = infile.readline()
    #skip sources

for node in np.random.choice([n for n in all_nodes], size=number_of_customers, replace=True):
    outfile.write(str(node) + "\n")

for line in infile:
    outfile.write(line)

infile.close()
outfile.close()
