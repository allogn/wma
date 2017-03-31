cmake_minimum_required(VERSION 3.7)
project(fcla)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ligraph -lboost_program_options")

#following for compiling osmtontw binary
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ligraph -lboost_program_options -lprotobuf-lite -losmpbf -lz")

include_directories(${CMAKE_SOURCE_DIR}/include/)

# BOOST libraries
find_package(Boost REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package( Boost COMPONENTS program_options REQUIRED )
include_directories(${Boost_INCLUDE_DIR})

#set (Boost_USE_STATIC_LIBS OFF) # enable dynamic linking
#set (Boost_USE_MULTITHREAD ON)  # enable multithreading
#find_package (Boost COMPONENTS REQUIRED chrono filesystem)

# include pbf reader
include_directories(${CMAKE_SOURCE_DIR}/lib/libosmpbfreader/)

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif()

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

set(IGRAPH_LIBS /usr/local/lib/libigraph.a)
include_directories(/usr/local/include/igraph/)

# setting executables
file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB TEST_SOURCE_FILES tests/*.cpp)
file(GLOB SIA_TEST_SOURCE_FILES tests/*.cpp)

add_executable(fcla main.cpp ${SOURCE_FILES};)
target_link_libraries(fcla ${Boost_LIBRARIES};${IGRAPH_LIBS})
add_executable(generator generator.cpp ${SOURCE_FILES};)
target_link_libraries(generator ${Boost_LIBRARIES};${IGRAPH_LIBS})

add_executable(brutesolver brutesolver.cpp)
target_link_libraries(brutesolver ${Boost_LIBRARIES};${IGRAPH_LIBS})

add_executable(SIA_tests tests/SIA_tests.cpp)
target_link_libraries(SIA_tests ${Boost_LIBRARIES};)

add_executable(fcla_tests tests/fcla_tests.cpp)
target_link_libraries(fcla_tests ${Boost_LIBRARIES};)

add_executable(infinite tests/infinite_tests.cpp)
target_link_libraries(infinite ${Boost_LIBRARIES};)

add_executable(simple_tests tests/simple_tests.cpp)
target_link_libraries(simple_tests ${Boost_LIBRARIES};${ZLIB_LIBRARIES};${PROTOBUF_LIBRARY})

add_executable(osmtontw osmtontw.cpp)
target_link_libraries(osmtontw ${Boost_LIBRARIES};${ZLIB_LIBRARIES};${PROTOBUF_LIBRARY})

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
