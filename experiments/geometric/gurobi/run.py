import os
import os.path as path
import subprocess

root_path = os.environ['FCLA_ROOT']
data_path = os.environ['DATA_PATH']

results = open("results.csv",'w')

for f in os.listdir(path.join(data_path,'geometric')):
    full_target_path = path.join(data_path,'geometric',f)
    print(full_target_path)

    edges = 0
    vertices = 0
    sources = 0

    # get size of a graph and number of customers
    with open(full_target_path) as check_file:
        params = check_file.readline().split(' ')
        edges = int(params[2])
        vertices = int(params[1])
        sources = int(params[3])

    capacity = 2
    facilities = sources


    p = subprocess.Popen(['python', path.join(root_path,'scripts','solveGurobi.py'),
                          full_target_path, str(capacity), str(facilities)],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    p.wait()
    out, err = p.communicate()
    results.write(out)