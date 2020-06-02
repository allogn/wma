#!/usr/bin/env bash

SIZE=10000
for SIZE_POW in {1..1..1};
do
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -c 40 -n ${SIZE} -o ${DATA_PATH}/clustered/n_m-n0.01c3_multi2/ -g 1 -s $(($SIZE*5/100)) --density 1.5 -u 1
    SIZE=$(($SIZE*2))
done
