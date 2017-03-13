#!/usr/bin/env bash

SIZE=16
for SIZE_POW in {1..7..1};
do
    ${FCLA_ROOT}/bin/generator -n 4096 -o ${DATA_PATH}/geometric/m_n4096k16c64a2 -g 0 -s $SIZE --density 2
    SIZE=$(($SIZE*2))
done
echo "Generation done."
