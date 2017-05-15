#!/usr/bin/env bash

SIZE=512
for SIZE_POW in {1..10..1};
do
    ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric_with_coords/n_m-n0.20_k-n0.1_c4_d2/ -g 0 -s $(($SIZE*2/10)) --density 2
    echo $SIZE
    SIZE=$(($SIZE*2))
done
echo "Generation done."
