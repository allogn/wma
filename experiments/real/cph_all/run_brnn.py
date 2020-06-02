#/usr/local/bin/python
import os
import os.path as path
import subprocess
import datetime

root_path = os.environ['FCLA_ROOT']
data_path = os.environ['DATA_PATH']
expdir = 'cph_multicap/all'
total = len(os.listdir(path.join(data_path,'real',expdir)))
count = 0
for f in os.listdir(path.join(data_path,'real',expdir)):
    if f[-4:] != '.ntw':
        continue
    count += 1
    full_target_path = path.join(data_path, 'real',expdir,f)
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

    #if vertices > 10000:
    #    continue

    facilities = int(sources*0.1)
    print("facilities " + str(facilities))
    capacity = 9
    for facilities in range(30,80,5):		  
        p = subprocess.Popen([path.join(root_path,'bin','nlrsolver'),'-i',
			  full_target_path,
			  '-c',str(capacity),
			  '-n',str(facilities),
                          '-f',path.join(data_path,'real', expdir, "facility_location_hours_full.csv"),
			  '-o',path.join(data_path,'real',expdir,'solutions','brnn',id + "fac" + str(facilities) + ".json")],
			  stdout=subprocess.PIPE,
			  stderr=subprocess.PIPE)
        p.wait()
        out, err = p.communicate()
        if len(out) + len(err) > 0:
	    print(out, err)
