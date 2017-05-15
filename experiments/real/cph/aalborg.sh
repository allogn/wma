#!/usr/bin/env bash

for CUST in {10..10000..100};
do
	echo $CUST
	../../bin/osmtontw -i ../aalborg.osm.pbf -c ${CUST} -o ./
done
