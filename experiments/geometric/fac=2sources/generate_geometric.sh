#!/usr/bin/env bash

for SIZE in {10..400..20};
do
    for ((SOURCE_NUM = 1; SOURCE_NUM <= $SIZE; SOURCE_NUM+=10));
    do
        ${FCLA_ROOT}/bin/generator -n ${SIZE} -o ${DATA_PATH}/geometric/fac\=2sources -g 0 -s ${SOURCE_NUM} --density 2
        echo "${SIZE}"
    done
done
echo "Generation done."
