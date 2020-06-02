#!/usr/bin/env bash

for TIMES in {1..1..1};
do
    ${FCLA_ROOT}/bin/generator -n $((10000-$TIMES)) -o ${DATA_PATH}/geometric_with_coords/greedy/ -g 0 -s 1000 --density 1.5
done
echo "Generation done."
