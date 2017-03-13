#!/usr/bin/env bash

for REP in {1..100..1};
do
    ${FCLA_ROOT}/bin/generator -n 4096 -o ${DATA_PATH}/geometric/perf -g 0 -s 256 --density 2
done
echo "Generation done."
