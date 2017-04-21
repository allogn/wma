Dependences:

- https://github.com/scrosby/OSM-binary and Google Protobuf (see C++ docs in github) for OSM parsing
- http://lemon.cs.elte.hu/trac/lemon/wiki/Downloads
- libigraph*
- gurobi python solver + gurobi itself

For osmtoontw remove comment in CMakeLists to link libraries.

Data formats:

- Results of optimization scripts are saved in json format to a file (one file per experiment)
- Each experiment === one graph, names and tagged by unique string id
- internal graph structure is saved in .ntw format, that is adjacency list and list of sources
- Full graph information is generated by getGraphInfo.py script and saved in a separate file

Experiment setup:

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

OSM downloads:
https://mapzen.com/data/metro-extracts/