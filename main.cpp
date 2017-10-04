/*
 * Facility matching main file
 */

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include "helpers.h"
#include "Network.h"
#include "FacilityChooser.h"
#include "igraph/igraph.h"
#include "Logger.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, const char** argv) {
    string filename;
    long facilities_to_locate;
    long facility_capacity;
    long lambda;
    double alpha;
    long gamma;
    string out_filename;
    string facilityfilename;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("input,i", po::value<string>(&filename)->required(), "Input file, a network")
            ("facilityfile,f", po::value<string>(&facilityfilename)->default_value(""), "List of potential facilities")
            ("facilities,n", po::value<long>(&facilities_to_locate)->required(), "Facilities to locate")
            ("faccap,c", po::value<long>(&facility_capacity)->default_value(1), "Capacity of facilities")
            ("lambda,l", po::value<long>(&lambda)->default_value(0), "Parameter lambda, set cover oversize")
            ("gamma,g", po::value<long>(&gamma)->default_value(0), "Parameter gamma, how many ties to select for set cover")
            ("alpha,a", po::value<double>(&alpha)->default_value(1), "Parameter alpha, exploring pace")
            ("output,o", po::value<string>(&out_filename)->required(), "Output file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    po::notify(vm);

    Logger logger;
    logger.start("total time");
    logger.start2("reading file");
    Network net(filename, facilityfilename);
    logger.finish2("reading file");

    FacilityChooser fcla(net, facilities_to_locate, facility_capacity, &logger, lambda, alpha, gamma);
    fcla.locateFacilities();
    fcla.calculateResult();
    switch(fcla.state) {
        case FacilityChooser::LOCATED:
            cout << logger.float_dict["objective"][0] << " " << logger.float_dict["runtime"][0] << endl;
            break;
        default:
            cout << "Error " << logger.str_dict["error"][0] << endl;
    }
    logger.finish("total time");
    logger.save(out_filename);
    return 0;
}
