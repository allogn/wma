#!/usr/bin/env bash

SIZE=256
for SIZE_POW in {1..10..1};
do
    ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric/n_m100k20c10a2 -g 0 -s 100 --density 2
    SIZE=$(($SIZE*2))
done
echo "Generation done."
