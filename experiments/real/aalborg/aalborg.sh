#!/usr/bin/env bash
CUST=65536
for CUSTNUM in {1..4..1};
do
	echo $CUST
	CUST=$(($CUST*2))
	../../../bin/osmtontw -i ../../../data/aalborg.osm.pbf -c ${CUST} -o ./generated/
done
