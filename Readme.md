Dependences:

- https://github.com/scrosby/OSM-binary and Google Protobuf (see C++ docs in github) for OSM parsing
- http://lemon.cs.elte.hu/trac/lemon/wiki/Downloads
- libigraph*
- gurobi python solver + gurobi itself

Export DATA_PATH variable before running!

Logging convention: experiment_id + number_of_facilities + facility_capacity + objVal + runtime
File format for graphs: .gr (including customers)