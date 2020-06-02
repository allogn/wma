#/usr/local/bin/python
import os
import os.path as path
import subprocess
import datetime

root_path = os.environ['FCLA_ROOT']
data_path = os.environ['DATA_PATH']
expdir = 'kc_n10000m1000'
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

    for facilities in range(20, 200, 30):
        print("facilities " + str(facilities))
        capacity = int(1.5*(sources/facilities+1))
    		  
        p = subprocess.Popen([path.join(root_path,'bin','hilbertsolver'),'-i',
	     		     full_target_path,
			     '-c',str(capacity),
			     '-n',str(facilities),
			     '-o',path.join(data_path,'geometric_with_coords',expdir,'solutions','hilbert',id + "_" + str(facilities) + ".json")],
			     stdout=subprocess.PIPE,
			     stderr=subprocess.PIPE)
        p.wait()
        out, err = p.communicate()
        if len(out) + len(err) > 0:
	   print(out, err)
