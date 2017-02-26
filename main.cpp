#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include "helpers.h"
#include "Network.h"
#include "FacilityChooser.h"
#include "igraph/igraph.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, const char** argv) {
    string filename;
    long facilities_to_locate;
    long facility_capacity;
    long lambda;
    string out_filename;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("input,i", po::value<string>(&filename)->required(), "Input file, a network")
            ("facilities,n", po::value<long>(&facilities_to_locate)->required(), "Facilities to locate")
            ("faccap,c", po::value<long>(&facility_capacity)->default_value(1), "Capacity of facilities")
            ("lambda,l", po::value<long>(&lambda)->default_value(0), "Parameter lambda, top-k facility heap threshold")
            ("output,o", po::value<string>(&out_filename)->required(), "Output file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    po::notify(vm);

    try {
        Network net(filename);
        FacilityChooser fcla(net, facilities_to_locate, facility_capacity, lambda);
        fcla.locateFacilities();
        fcla.calculateResult();
        fcla.saveResult(out_filename);
    } catch (string e) {
        if (e == "Trivial solution: Too many facilities") {
            cout << 0 << endl;
        } else {
            cout << e << endl;
        }
    };

    return 0;
}
