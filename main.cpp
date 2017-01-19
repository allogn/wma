#include <iostream>
#include <RoadNetwork.h>

#include "helpers.h"
#include "igraph/igraph.h"

using namespace std;

int main() {
    RoadNetwork r;
    r.load_pbf("../../../data/riga_latvia.osm.pbf");
    cout << igraph_vcount(&r.graph) << endl;
    return 0;
}