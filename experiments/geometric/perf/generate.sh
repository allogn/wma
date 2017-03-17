#!/usr/bin/env bash

for REP in {1..1..1};
do
    ${FCLA_ROOT}/bin/generator -n 256 -o ./ -g 0 -s 8 --density 2
done
echo "Generation done."
