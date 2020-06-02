#!/bin/bash
GRAPH_DATA="clustered/n_m-n0.05k-n0.01c20d2conClust20"
for filename in $DATA_PATH/$GRAPH_DATA/*.ntw; do
	echo $filename
	python $FCLA_ROOT/scripts/calculateDistMatrSTA.py $filename None $DATA_PATH/$GRAPH_DATA/$(basename $filename)
done
