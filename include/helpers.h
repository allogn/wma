//
// Created by allogn on 17.01.17.
//

#ifndef FCLA_HELPERS_H
#define FCLA_HELPERS_H

#include <string>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include "igraph/igraph.h"

// check if file exists
inline bool file_exists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

typedef std::pair<float, float> coords;

#endif //FCLA_HELPERS_H
