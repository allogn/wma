#!/usr/bin/env bash

for SIZE in {100..1000..30};
do
    ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric/30sourcesVarySize -g 0 -s 30 --density 2
done
echo "Generation done."
