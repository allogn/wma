#/usr/local/bin/python
import os
import os.path as path
import subprocess
import datetime

root_path = os.environ['FCLA_ROOT']
data_path = os.environ['DATA_PATH']
expdir = 'c_n10000m1000k100cl20d1.5'
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
    fac = 200

    for capacity in range(8,15,1):
        print(capacity)
        p = subprocess.Popen(['python', path.join(root_path,'scripts','solveGurobi.py'),
                          full_target_path,
                          str(capacity),
                          str(fac),
                          path.join(data_path,'clustered',expdir,'solutions','gurobi',id + "_" + str(capacity) + ".json")],
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
        p.wait()
        out, err = p.communicate()
    if len(out) + len(err) > 0:
        print(out, err)
