cmake_minimum_required(VERSION 3.5)
project(fcla)
set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_SOURCE_DIR}/include/)

# BOOST libraries
find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
endif()
#set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
#set (Boost_USE_MULTITHREAD ON)  # enable multithreading
#find_package (Boost COMPONENTS REQUIRED chrono filesystem)

# include igraph
set(IGRAPH_BUILD_PATH ${CMAKE_SOURCE_DIR}/lib/igraph-0.7.1/build/)
set(IGRAPH_LINK_LIBS ${IGRAPH_BUILD_PATH}/lib/libigraph.so)
include_directories(${IGRAPH_BUILD_PATH}/include/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# setting executables
file(GLOB SOURCE_FILES src/*.cpp)

add_executable(fcla main.cpp ${SOURCE_FILES})
target_link_libraries (fcla ${Boost_LIBRARIES})