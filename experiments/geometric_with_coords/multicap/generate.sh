#!/usr/bin/env bash

SIZE=512
for SIZE_POW in {1..10..1};
do
    ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric_with_coords/multicap/ -g 0 -s $(($SIZE*1/10)) --density 1.4
    SIZE=$(($SIZE*2))
    echo $SIZE
done
echo "Generation done."
