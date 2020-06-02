#!/bin/bash
GRAPH_DATA="geometric_with_coords/n_m-n0.20_k-n0.1_c4_d2"
for filename in $DATA_PATH/$GRAPH_DATA/*.ntw; do
	echo $filename
	python $FCLA_ROOT/scripts/calculateDistMatrATA.py $filename None $DATA_PATH/$GRAPH_DATA/$(basename $filename)
done
