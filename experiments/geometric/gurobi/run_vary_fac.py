#!/usr/local/bin/python
import os
import os.path as path
import subprocess
import datetime

root_path = os.environ['FCLA_ROOT']
data_path = os.environ['DATA_PATH']
total = len(os.listdir(path.join(data_path,'geometric','100-300:sources30')))
count = 0
for f in os.listdir(path.join(data_path,'geometric','100-300:sources30')):
    if f[-4:] != '.ntw':
        continue
    count += 1
    full_target_path = path.join(data_path,'geometric','100-300:sources30',f)
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
    if (sources < 300):
        continue
    capacity = 5
    for facilities in range(6,30):

        p = subprocess.Popen(['python', path.join(root_path,'scripts','solveGurobi.py'),
                          full_target_path,
                          str(capacity),
                          str(facilities),
                          path.join(data_path,'solutions','geometric','300:sources30:varfac','gurobi',id + ".json")],
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
        p.wait()
        out, err = p.communicate()
        if len(out) + len(err) > 0:
            print(out, err)
