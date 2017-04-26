#!/usr/bin/env bash

SIZE=1000
for SIZE_POW in {1..10..1};
do
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -c 40 -n ${SIZE} -o ${DATA_PATH}/clustered/n_m-n0.05c-minc2m-k10/ -g 1 -s $(($SIZE*5/100)) --density 1.5
    SIZE=$(($SIZE+1000))
done
echo "Generation done."
