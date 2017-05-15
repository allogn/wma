#!/usr/bin/env bash

SIZE=1024
for SIZE_POW in {1..7..1};
do
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -c 20 -n ${SIZE} -o ${DATA_PATH}/clustered/n_m-n0.05k-n0.01c20d1conClust20/ -g 1 -s $(($SIZE*5/100)) --density 1
    SIZE=$(($SIZE*2))
done
echo "Generation done."
