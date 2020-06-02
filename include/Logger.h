

#ifndef FCLA_LOGGER_H
#define FCLA_LOGGER_H

#ifndef _DEBUG_
#define _DEBUG_ 1
#endif

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <ctime>
#include <vector>
#include <iomanip>
#include <chrono>

class Logger {
public:
    std::map<std::string, std::vector<std::string>> str_dict;
    std::map<std::string, std::vector<double>> float_dict;
    std::map<std::string, clock_t> opened_times;

    Logger() {}
    ~Logger() {}

    inline void add(std::string key, std::string val) {
        std::map<std::string, std::vector<std::string>>::iterator it;
        if (it != str_dict.end()) {
            str_dict[key].push_back(val);
        } else {
            str_dict[key] = std::vector<std::string>({val});
        }
    }
    inline void add(std::string key, double val) {
        std::map<std::string, std::vector<double>>::iterator it;
        if (it != float_dict.end()) {
            float_dict[key].push_back(val);
        } else {
            float_dict[key] = std::vector<double>({val});
        }
    }

    inline void add1(std::string key, std::string val) {
//#if _DEBUG_ > 0
        std::map<std::string, std::vector<std::string>>::iterator it;
        if (it != str_dict.end()) {
            str_dict[key].push_back(val);
        } else {
            str_dict[key] = std::vector<std::string>({val});
        }
//#endif
    }
    inline void add1(std::string key, double val) {
//#if _DEBUG_ > 0
        std::map<std::string, std::vector<double>>::iterator it;
        if (it != float_dict.end()) {
            float_dict[key].push_back(val);
        } else {
            float_dict[key] = std::vector<double>({val});
        }
//#endif
    }

    inline void add2(std::string key, std::string val) {
//#if _DEBUG_ > 1
        std::map<std::string, std::vector<std::string>>::iterator it;
        if (it != str_dict.end()) {
            str_dict[key].push_back(val);
        } else {
            str_dict[key] = std::vector<std::string>({val});
        }
//#endif
    }
    inline void add2(std::string key, double val) {
//#if _DEBUG_ > 1
        std::map<std::string, std::vector<double>>::iterator it;
        if (it != float_dict.end()) {
            float_dict[key].push_back(val);
        } else {
            float_dict[key] = std::vector<double>({val});
        }
//#endif
    }

    /*
     * Timeit
     */
    inline void start(std::string key) {
        clock_t begin = clock();
        opened_times[key] = begin;
    }

    inline void finish(std::string key) {
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

    inline void start1(std::string key) {
//#if _DEBUG_ > 0
        clock_t begin = clock();
        opened_times[key] = begin;
//#endif
    }

    inline void finish1(std::string key) {
//#if _DEBUG_ > 0
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
//#endif
    }

    inline void start2(std::string key) {
//#if _DEBUG_ > 1
        clock_t begin = clock();
        opened_times[key] = begin;
//#endif
    }

    inline void finish2(std::string key) {
//#if _DEBUG_ > 1
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
//#endif
    }

    void save(std::string out_filename) {
        std::ofstream outf(out_filename, std::ios::out);
        outf << "{";

        for (auto it = str_dict.begin(); it != str_dict.end(); it++) {
            if (it->second.size() == 1) {
                outf << "\"" << it->first << "\":\"" << it->second[0] << "\",\n";
            } else {
                outf << "\"" << it->first << "\":[";
                for (long i = 0; i < it->second.size()-1; i++) {
                    outf << "\"" << it->second[i] << "\",\n";
                }
                outf << "\"" << it->second[it->second.size()-1] << "\"],\n";
            }
        }
        for (auto it = float_dict.begin(); it != float_dict.end(); it++) {
            if (it->second.size() == 1) {
                long res = it->second[0];
                if ((double) res == it->second[0]) {
                            outf << "\"" << it->first << "\":" << res << ",\n";
                } else {
                            outf << "\"" << it->first << "\":" << it->second[0] << ",\n";
                }
            } else {
                outf << "\"" << it->first << "\":[";
                for (long i = 0; i < it->second.size()-1; i++) {
                    outf << it->second[i] << ",\n";
                }
                outf << it->second[it->second.size()-1] << "],\n";
            }
        }

        std::chrono::time_point<std::chrono::system_clock> now;
        now = std::chrono::system_clock::now();
        std::time_t end_time = std::chrono::system_clock::to_time_t(now);
        std::string tt = std::string(std::ctime(&end_time));
        tt = tt.substr(0,tt.size()-1);
        outf << "\"Created\":\"" << tt << "\"\n";
        outf << "}";
        outf.close();
    }
};

#endif //FCLA_LOGGER_H
