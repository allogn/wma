# Facility Location Algorithm

Algorithm for large-scale approximate capacitated facility location. Locates a set of facilities inside a road network, minimizing the sum of distances from each customer to the nearest available facility.

Codebase of the publication

A. Logins, P. Karras, C. S. Jensen, "Multicapacity Facility Selection in Networks", In Proceedings of the 35th IEEE International Conference on Data Engineering (ICDE 2019), Macao, Macao SAR, April 2019, pp. 794–805. ([pdf](https://alogins.net/assets/pdf/wma_icde2019.pdf))

## Dependencies

- https://github.com/scrosby/OSM-binary and Google Protobuf (see C++ docs in github) for OSM parsing (osmtontw), can be swiched off by passing a parameter -DOSM to cmake
- http://lemon.cs.elte.hu/trac/lemon/wiki/Downloads
- libigraph\*

For experimental evaluation, Gurobi is used as a convex solver. Scripts folder contain a script for running Gurobi for all graphs in particular folder.
- gurobi python solver + gurobi itself

## Data formats

- Results of optimization scripts are saved in json format to a file (one file per experiment)
- Each experiment === one graph, names and tagged by unique string id
- internal graph structure is saved in .ntw format, that is adjacency list and list of sources
- Full graph information is generated by getGraphInfo.py script and saved in a separate file

Graphs are stored in internal graph representation, called .ntw

## Installation

```bash
# Debug level: 0 - no checks, 1 - feasibility check, 2 - statistics output
# If Google Protobuf is installed, use -DOSM=ON
cmake -D DEBUG={0,1,2} -DOSM_LIBS={ON,OFF} ./ 
```

## Experiment setup

- set up environmental variables FCLA_ROOT and DATA_PATH
- when running cmake, specify debug level by -D DEBUG=0..5, where 1 is basic checks, 2 is stats
- set up gurobi environmental variables to point to installation dir, note that bash script can not export variable
- generate data by script in experiments/<type of data> folder
- create run.py script in experiments folder (<type of data>/<algorithm>/run.py)
- create $DATA_PATH/solutions/<type of data>/<algorithm>/ folder
- check the version of script / compile binary
- create ipynb notebook in experiments/analysis folder
- load data by scripts/mergeResults.py script, save resulting dataframe in temporal File in experiments folder
- plot and analyze

## Links 
OSM downloads:
https://mapzen.com/data/metro-extracts/
