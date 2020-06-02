#!/bin/bash
GRAPH_DATA="clustered/n_m-n0.1k-n0.01c20d2conClust5"
for filename in $DATA_PATH/$GRAPH_DATA/*.ntw; do
	echo $filename
	python $FCLA_ROOT/scripts/calculateDistMatrATA.py $filename None $DATA_PATH/$GRAPH_DATA/$(basename $filename)
done
