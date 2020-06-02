#/usr/local/bin/python
import os
import os.path as path
import subprocess
import datetime

root_path = os.environ['FCLA_ROOT']
data_path = os.environ['DATA_PATH']
expdir = 'multicap'
total = len(os.listdir(path.join(data_path,'geometric_with_coords',expdir)))
count = 0
for f in os.listdir(path.join(data_path,'geometric_with_coords',expdir)):
    if f[-4:] != '.ntw':
        continue
    count += 1
    full_target_path = path.join(data_path,'geometric_with_coords',expdir,f)
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

    if vertices > 1100:
        continue

    facilities = int(sources/2)
    capacity = 10

    file_with_capacities = path.join(data_path, 'geometric_with_coords', expdir, f[:-4] + '.csv')
    p = subprocess.Popen([path.join(root_path,'bin','fcla'),'-i',
			  full_target_path,
			  '-c',str(capacity),
			  '-f',file_with_capacities,
			  '-g','1',
			  '-n',str(facilities),
			  '-o',path.join(data_path,'geometric_with_coords',expdir,'solutions','fcla_g_rand',id + "fac" + str(facilities) + ".json")],
			  stdout=subprocess.PIPE,
			  stderr=subprocess.PIPE)
    p.wait()
    out, err = p.communicate()
    if len(out) + len(err) > 0:
        print(out, err)
