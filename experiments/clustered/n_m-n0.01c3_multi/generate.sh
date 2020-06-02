#!/usr/bin/env bash

SIZE=10000
for SIZE_POW in {1..1..1};
do
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -c 50 -n ${SIZE} -o ${DATA_PATH}/clustered/n_m-n0.01c3_multi/ -g 1 -s $(($SIZE*1/10)) --density 1.5 -u 1
    SIZE=$(($SIZE*2))
done
