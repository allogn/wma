cmake_minimum_required(VERSION 3.5)
project(fcla)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ligraph")

include_directories(${CMAKE_SOURCE_DIR}/include/)

# BOOST libraries
find_package(Boost REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
endif()

#set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
#set (Boost_USE_MULTITHREAD ON)  # enable multithreading
#find_package (Boost COMPONENTS REQUIRED chrono filesystem)

# include igraph
#set(IGRAPH_BUILD_PATH ${CMAKE_SOURCE_DIR}/lib/igraph-0.7.1/build/)
#set(IGRAPH_LINK_LIBS ${IGRAPH_BUILD_PATH}/lib/libigraph.so)
#include_directories(${IGRAPH_BUILD_PATH}/include/)
#include_directories()
#set(IGRAPH_INCLUDE_DIR)

# include pbf reader
include_directories(${CMAKE_SOURCE_DIR}/lib/OSM-binary/build/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libosmpbfreader/)
include_directories(${CMAKE_SOURCE_DIR}/lib/lemon-1.3.1/lib/include)
set(OSMPBF_LINK_LIBS ${CMAKE_SOURCE_DIR}/lib/OSM-binary/build/lib/libosmpbf.so)

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif()

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})


# setting executables
file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB TEST_SOURCE_FILES tests/*.cpp)
file(GLOB SIA_TEST_SOURCE_FILES tests/*.cpp)

add_executable(fcla main.cpp ${SOURCE_FILES})
target_link_libraries(fcla ${Boost_LIBRARIES};${IGRAPH_LINK_LIBS};${OSMPBF_LINK_LIBS};${ZLIB_LIBRARIES};${PROTOBUF_LIBRARY})

add_executable(simple_tests tests/simple_tests.cpp)
target_link_libraries(simple_tests ${Boost_LIBRARIES};${IGRAPH_LINK_LIBS};${OSMPBF_LINK_LIBS};${ZLIB_LIBRARIES};${PROTOBUF_LIBRARY})

add_executable(SIA_tests tests/SIA_tests.cpp)
target_link_libraries(SIA_tests ${Boost_LIBRARIES};)#${IGRAPH_LINK_LIBS};${LEMON_LINK_LIBS})

add_executable(fcla_tests tests/fcla_tests.cpp)
target_link_libraries(fcla_tests ${Boost_LIBRARIES};)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)