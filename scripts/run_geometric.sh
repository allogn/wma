#!/usr/bin/env bash

CAPACITY=3
NUM_OF_FACILITIES=4

rm ../temp/*

for f in ../data/*_30_*;
do
    echo "Processing $f"
    echo "$f" >> ../temp/file_list.txt
    ../bin/fcla -i $f -n $NUM_OF_FACILITIES -c $CAPACITY -l 0 >> ../temp/fcla_answers.txt
    python solveGurobi.py $f $CAPACITY $NUM_OF_FACILITIES >> ../temp/gurobi_answers.txt
    ../bin/brutesolver -i $f -n $NUM_OF_FACILITIES -c $CAPACITY >> ../temp/brute_answers.txt
done
