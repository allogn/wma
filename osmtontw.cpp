//
// Created by Alvis Logins on 21/03/2017.
//

/*
 * Facility matching main file
 */


#define _DEBUG_ 10

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include "helpers.h"
#include "Network.h"
#include "RoadNetwork.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, const char** argv) {
    string filename;
    long customers_to_locate;
    string out_filename;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("input,i", po::value<string>(&filename)->required(), "Input file, OSM file")
            ("customers,c", po::value<long>(&customers_to_locate)->required(), "Customers to locate")
            ("output,o", po::value<string>(&out_filename)->required(), "Output directory (name automatic)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    po::notify(vm);

    RoadNetwork network;
    network.load_pbf(filename);
    network.transform_coordinates();
    network.make_weights();
    network.save_network(out_filename, customers_to_locate);

    return 0;
}
