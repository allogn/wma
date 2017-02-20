#!/usr/bin/env bash

for SIZE in {10..200..20};
do
    for SOURCE_NUM in {1..10..1};
    do
        echo ../bin/generator -n ${SIZE} -o ../data/geometric_${SIZE}_${SOURCE_NUM}.gr -g 0 -s ${SOURCE_NUM} --density 0.4
        ../bin/generator -n ${SIZE} -o ../data/geometric_${SIZE}_${SOURCE_NUM}.gr -g 0 -s ${SOURCE_NUM} --density 0.4
    done
done
echo "Generation done"
