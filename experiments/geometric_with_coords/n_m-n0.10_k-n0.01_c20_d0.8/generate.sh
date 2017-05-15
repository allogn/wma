#!/usr/bin/env bash

SIZE=512
for SIZE_POW in {1..10..1};
do
    ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric_with_coords/n_m-n0.10_k-n0.01_c20_d0.8/ -g 0 -s $(($SIZE*1/10)) --density 1.2
    SIZE=$(($SIZE*2))
done
echo "Generation done."
