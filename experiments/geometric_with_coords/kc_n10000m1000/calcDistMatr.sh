#!/bin/bash
GRAPH_DATA="clustered/kc_n10000m1000cl40"
for filename in $DATA_PATH/$GRAPH_DATA/*.ntw; do
	echo $filename
	python $FCLA_ROOT/scripts/calculateDistMatrSTA.py $filename None $DATA_PATH/$GRAPH_DATA/$(basename $filename)
done
