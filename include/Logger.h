

#ifndef FCLA_LOGGER_H
#define FCLA_LOGGER_H

#include <iostream>
#include <map>
#include <string>
#include <ctime>
#include <vector>
#include <iomanip>

class Logger {
public:
    std::map<std::string, std::vector<std::string>> str_dict;
    std::map<std::string, std::vector<double>> float_dict;
    std::map<std::string, std::vector<long>> int_dict;
    std::map<std::string, clock_t> opened_times;

    Logger() {}
    ~Logger() {}

    void add(std::string key, std::string val) {
        std::map<std::string, std::vector<std::string>>::iterator it;
        if (it != str_dict.end()) {
            str_dict[key].push_back(val);
        } else {
            str_dict[key] = std::vector<std::string>({val});
        }
    }
    void add(std::string key, double val) {
        std::map<std::string, std::vector<double>>::iterator it;
        if (it != float_dict.end()) {
            float_dict[key].push_back(val);
        } else {
            float_dict[key] = std::vector<double>({val});
        }
    }
    void add(std::string key, long val) {
        std::map<std::string, std::vector<long>>::iterator it;
        if (it != int_dict.end()) {
            int_dict[key].push_back(val);
        } else {
            int_dict[key] = std::vector<long>({val});
        }
    }

    /*
     * Timeit
     */
    void start(std::string key) {
        clock_t begin = clock();
        opened_times[key] = begin;
    }

    void finish(std::string key) {
        clock_t end = clock();
        clock_t begin = opened_times[key];
        double elapsed = double(end - begin) / CLOCKS_PER_SEC;

        std::map<std::string, std::vector<double>>::iterator it;
        it = float_dict.find(key);
        if (it != float_dict.end()) {
            float_dict[key].push_back(elapsed);
        } else {
            float_dict[key] = std::vector<double>({elapsed});
        }
    }

    void save(std::string out_filename) {
        std::ofstream outf(out_filename, std::ios::out);
        outf << "{";

        for (auto it = str_dict.begin(); it != str_dict.end(); it++) {
            if (it->second.size() == 1) {
                outf << "\"" << it->first << "\":\"" << it->second[0] << "\",";
            } else {
                outf << "\"" << it->first << "\":[";
                for (long i = 0; i < it->second.size()-1; i++) {
                    outf << "\"" << it->second[i] << "\",";
                }
                outf << "\"" << it->second[it->second.size()-1] << "\"],";
            }
        }
        for (auto it = float_dict.begin(); it != float_dict.end(); it++) {
            if (it->second.size() == 1) {
                outf << "\"" << it->first << "\":" << it->second[0] << ",";
            } else {
                outf << "\"" << it->first << "\":[";
                for (long i = 0; i < it->second.size()-1; i++) {
                    outf << it->second[i] << ",";
                }
                outf << it->second[it->second.size()-1] << "],";
            }
        }
        for (auto it = int_dict.begin(); it != int_dict.end(); it++) {
            if (it->second.size() == 1) {
                outf << "\"" << it->first << "\":" << it->second[0] << ",";
            } else {
                outf << "\"" << it->first << "\":[";
                for (long i = 0; i < it->second.size()-1; i++) {
                    outf << it->second[i] << ",";
                }
                outf << it->second[it->second.size()-1] << "],";
            }
        }

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        outf << "\"Created\":\"" << std::put_time(std::localtime(&now_c), "%F %T") << "\"";
        outf << "}";
    }
};

#endif //FCLA_LOGGER_H
