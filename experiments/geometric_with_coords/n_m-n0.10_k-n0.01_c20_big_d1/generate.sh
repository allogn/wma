#!/usr/bin/env bash

SIZE=1024
for SIZE_POW in {1..10..1};
do
    ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric_with_coords/n_m-n0.10_k-n0.01_c20_big_d1/ -g 0 -s $(($SIZE*1/10)) --density 1
    echo ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric_with_coords/n_m-n0.10_k-n0.01_c20_big_d1/ -g 0 -s $(($SIZE*1/10)) --density 1
    SIZE=$(($SIZE*2))
done
echo "Generation done."
