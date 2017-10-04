#!/usr/bin/env bash
CUST=2
for CUST_POW in {1..16..1};
do
	echo $CUST
	../../bin/osmtontw -i ../../data/riga.osm.pbf -c ${CUST} -o ./generated/
	CUST=$((2*$CUST))
done
