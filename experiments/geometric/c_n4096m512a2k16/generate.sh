#!/usr/bin/env bash

${FCLA_ROOT}/bin/generator -n 4096 -o ${DATA_PATH}/geometric/c_n4096m512a2k16 -g 0 -s 512 --density 2
echo "Generation done."
