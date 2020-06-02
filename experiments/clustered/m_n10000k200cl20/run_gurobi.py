#/usr/local/bin/python
import os
import os.path as path
import subprocess
import datetime

root_path = os.environ['FCLA_ROOT']
data_path = os.environ['DATA_PATH']
expdir = 'm_n10000k200cl20'
total = len(os.listdir(path.join(data_path,'clustered',expdir)))
count = 0
for f in os.listdir(path.join(data_path,'clustered',expdir)):
    if f[-4:] != '.ntw':
        continue
    count += 1
    full_target_path = path.join(data_path,'clustered',expdir,f)
    print(" ".join((str(datetime.datetime.now()),full_target_path,"(",str(count),"/",str(total),")")))

    edges = 0
    vertices = 0
    sources = 0

    # get size of a graph and number of customers
    with open(full_target_path) as check_file:
        params = check_file.readline().split(' ')
        id = params[0]
        edges = int(params[2])
        vertices = int(params[1])
        sources = int(params[3])
    facilities = 128
    capacity = 10*(sources/facilities + 1)
    #if sources > 2**12 or sources < 2**12:
    #    continue
    p = subprocess.Popen(['python', path.join(root_path,'scripts','solveGurobi.py'),
                          full_target_path,
                          str(capacity),
                          str(facilities),
                          path.join(data_path,'clustered',expdir,'solutions','gurobi',id + "_" + str(capacity) + ".json"),
			  "",
			  path.join(data_path,"clustered",expdir,"distmatr"+str(capacity)+".pkl")], #same distance matrix for all files, only sources differ --- I guess there is a mistake, so no, there are multiple files
                          stdout=subprocess.PIPE, 
                          stderr=subprocess.PIPE)
    p.wait()
    out, err = p.communicate()
    if len(out) + len(err) > 0:
        print(out, err)
