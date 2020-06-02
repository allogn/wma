#!/usr/bin/env bash


for SIZE1 in {1..2..1};
do
    for SIZE2 in {0..9..2};
    do
        echo $SIZE1
        ${FCLA_ROOT}/bin/generator -c 40 -n 10000 -o ${DATA_PATH}/clustered/d_n10000-m1000-cl40-k300-c10/ -g 1 -s 1000 --density ${SIZE1}.${SIZE2}
    done
done
echo "Generation done."
