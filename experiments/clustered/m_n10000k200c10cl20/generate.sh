#!/usr/bin/env bash

for SIZE in {200..1000..100};
do
    echo $SIZE
    ${FCLA_ROOT}/bin/generator -c 20 -n 10000 -o ${DATA_PATH}/clustered/m_n10000k200c10cl20/ -g 1 -s $SIZE --density 1.5
done
echo "Generation done."
