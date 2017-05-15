#!/usr/bin/env bash

SIZE=10000
for SIZE_POW in {1..1..1};
do
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -c 20 -n ${SIZE} -o ${DATA_PATH}/clustered/k_n10000m1000c10cl20/ -g 1 -s 1000 --density 1.5
    SIZE=$(($SIZE*2))
done
echo "Generation done."
