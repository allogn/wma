#!/bin/bash
export $FCLA_ROOT=$PWD

# igraph Library
cd $FCLA_ROOT/lib/igraph-0.7.1/
./configure --prefix=$FCLA_ROOT/lib/igraph-0.7.1/build
make
make install


# OSM-binary
cd $FCLA_ROOT/lib/OSM-binary/
export --prefix=$FCLA_ROOT/lib/OSM-binary/build
make -C src
make -C src install

