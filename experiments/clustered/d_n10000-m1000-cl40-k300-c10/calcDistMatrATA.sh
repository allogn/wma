#!/bin/bash
GRAPH_DATA="clustered/d_n10000-m1000-cl40-k300-c10"
for filename in $DATA_PATH/$GRAPH_DATA/*.ntw; do
	echo $filename
	python $FCLA_ROOT/scripts/calculateDistMatrATA.py $filename None $DATA_PATH/$GRAPH_DATA/$(basename $filename)
done
