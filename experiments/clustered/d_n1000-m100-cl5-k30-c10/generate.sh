#!/usr/bin/env bash


for SIZE1 in {1..3..1};
do
    for SIZE2 in {0..9..2};
    do
        echo $SIZE1
        ${FCLA_ROOT}/bin/generator -c 5 -n 1000 -o ${DATA_PATH}/clustered/d_n1000-m100-cl5-k30-c10/ -g 1 -s 100 --density ${SIZE1}.${SIZE2}
    done
done
echo "Generation done."
