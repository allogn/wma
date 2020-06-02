#!/bin/bash
GRAPH_DATA="clustered/k_n10000m1000c10cl20"
for filename in $DATA_PATH/$GRAPH_DATA/*.ntw; do
	echo $filename
	python $FCLA_ROOT/scripts/calculateDistMatrATA.py $filename None $DATA_PATH/$GRAPH_DATA/$(basename $filename)
done
