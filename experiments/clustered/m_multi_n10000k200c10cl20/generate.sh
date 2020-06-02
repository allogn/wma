#!/usr/bin/env bash
SIZE=256
for SIZEPOW in {1..1};
do
    SIZE=$(($SIZE*2))
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -r 1 -c 21 -n 10000 -o ${DATA_PATH}/clustered/m_multi_n10000k200c10cl20/ -g 1 -s $SIZE --density 1.5
done
echo "Generation done."
