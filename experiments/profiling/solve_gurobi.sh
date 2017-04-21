#!/bin/bash

for f in many/*;
do
	python ../../../scripts/solveGurobi.py $f 10 6 ./many/solutions/gurobi/$f.json
done;
