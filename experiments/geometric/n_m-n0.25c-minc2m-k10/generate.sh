#!/usr/bin/env bash

SIZE=1000
for SIZE_POW in {1..10..1};
do
    ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric/n_m-n0.25c-minc2m-k10 -g 0 -s $(($SIZE*25/100)) --density 2
    SIZE=$(($SIZE+1000))
done
echo "Generation done."
