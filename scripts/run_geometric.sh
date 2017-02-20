#!/usr/bin/env bash

for f in ../data/*;
do
    echo "Processing $f"
    ../bin/fcla -i $f -n 4 -c 3 -l 0 -o temp.txt
done
