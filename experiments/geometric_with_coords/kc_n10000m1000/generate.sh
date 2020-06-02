#!/usr/bin/env bash

SIZE=10000
for SIZE_POW in {1..1..1};
do
    ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric_with_coords/kc_n10000m1000/ -g 0 -s 100 --density 1.5
done
echo "Generation done."
