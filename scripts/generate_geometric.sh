#!/usr/bin/env bash
DATA_DIR=/q/storage/alogins/graphs/random_geometric
for SIZE in {10..400..50};
do
    for ((SOURCE_NUM = 1; SOURCE_NUM <= $SIZE; SOURCE_NUM+=10));
    do
        ../bin/generator -n ${SIZE} -o ${DATA_DIR}/geometric_${SIZE}_${SOURCE_NUM}.gr -g 0 -s ${SOURCE_NUM} --density $(($SIZE/100))
    done
done
echo "Generation done"
