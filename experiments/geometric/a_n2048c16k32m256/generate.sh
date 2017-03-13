#!/usr/bin/env bash

SIZE=15
for SIZE_POW in {1..10..1};
do
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -n 2048 -o ${DATA_PATH}/geometric/a_n2048c16k32m256 -g 0 -s 256 --density $(echo "$SIZE/10"|bc -l)
    SIZE=$(($SIZE+5))
done
echo "Generation done."
