#!/bin/bash
GRAPH_DATA="clustered/n_m-n0.05c-minc2m-k10"
for filename in $DATA_PATH/$GRAPH_DATA/*.ntw; do
	echo $filename
	python $FCLA_ROOT/scripts/calculateDistMatrSTA.py $filename None $DATA_PATH/$GRAPH_DATA/$(basename $filename)
done
