#!/usr/bin/env bash
SIZE=128
for SIZEPOW in {1..5};
do
    SIZE=$(($SIZE*2))
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -r 1 -c 21 -n 10000 -o ${DATA_PATH}/clustered/m_n10000k200cl20/ -g 1 -s $SIZE --density 1.5
done
echo "Generation done."
