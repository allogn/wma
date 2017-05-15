#!/usr/bin/env bash

SIZE=1024
for SIZE_POW in {1..9..1};
do
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -c 50 -n ${SIZE} -o ${DATA_PATH}/clustered/n_m-n0.01c3/ -g 1 -s $(($SIZE*1/100)) --density 0.5
    SIZE=$(($SIZE*2))
done
echo "Generation done."
