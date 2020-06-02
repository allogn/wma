#!/usr/bin/env bash

for TIMES in {1..1..1};
do
    ${FCLA_ROOT}/bin/generator -c 20 -n 10000 -o ${DATA_PATH}/clustered/greedy/ -g 1 -s 1000 -u 1 --density 1.0
done
echo "Generation done."
