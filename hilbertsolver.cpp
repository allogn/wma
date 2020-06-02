/*
 * Facility matching hilbert solver file.
 *
 * There could be not more than 1 facility per node, but multiple customers per node are allowed.
 */


#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "HilbertSolver.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, const char** argv) {
    string filename;
    long facility_number_to_locate;
    long facility_capacity;
    string out_filename;
    string facilityfile;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("input,i", po::value<string>(&filename)->required(), "Input file, a network")
            ("facilityfile,f", po::value<string>(&facilityfile)->default_value(""), "File with a list of facilities")
            ("facilities,n", po::value<long>(&facility_number_to_locate)->required(), "Facilities to locate")
            ("faccap,c", po::value<long>(&facility_capacity)->default_value(1), "Capacity of facilities")
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
//    if (facilityfile != "") {
        //@todo if components > 1 - special case
        //@todo change uniform capacities
//        throw "Not implemented";
//    }
    try {
        Network net(filename,facilityfile);
        HilbertSolver hilbert_solver = HilbertSolver(&net, &logger);
        hilbert_solver.run(facility_number_to_locate, facility_capacity);
        if (logger.str_dict.count("error") > 0) {
            cout << "Error " << logger.str_dict["error"][0] << endl;
        } else {
            cout << logger.float_dict["objective"][0] << " " << logger.float_dict["runtime"][0] << endl;
        }
        logger.finish("total time");
        hilbert_solver.save_log(out_filename);
    } catch (const std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}
