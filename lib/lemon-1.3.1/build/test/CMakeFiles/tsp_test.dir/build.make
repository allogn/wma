# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.7.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.7.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/alvis/PhD/fcla/lib/lemon-1.3.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build

# Include any dependencies generated for this target.
include test/CMakeFiles/tsp_test.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/tsp_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/tsp_test.dir/flags.make

test/CMakeFiles/tsp_test.dir/tsp_test.cc.o: test/CMakeFiles/tsp_test.dir/flags.make
test/CMakeFiles/tsp_test.dir/tsp_test.cc.o: ../test/tsp_test.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/tsp_test.dir/tsp_test.cc.o"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/test && /Library/Developer/CommandLineTools/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tsp_test.dir/tsp_test.cc.o -c /Users/alvis/PhD/fcla/lib/lemon-1.3.1/test/tsp_test.cc

test/CMakeFiles/tsp_test.dir/tsp_test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tsp_test.dir/tsp_test.cc.i"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/test && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alvis/PhD/fcla/lib/lemon-1.3.1/test/tsp_test.cc > CMakeFiles/tsp_test.dir/tsp_test.cc.i

test/CMakeFiles/tsp_test.dir/tsp_test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tsp_test.dir/tsp_test.cc.s"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/test && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alvis/PhD/fcla/lib/lemon-1.3.1/test/tsp_test.cc -o CMakeFiles/tsp_test.dir/tsp_test.cc.s

test/CMakeFiles/tsp_test.dir/tsp_test.cc.o.requires:

.PHONY : test/CMakeFiles/tsp_test.dir/tsp_test.cc.o.requires

test/CMakeFiles/tsp_test.dir/tsp_test.cc.o.provides: test/CMakeFiles/tsp_test.dir/tsp_test.cc.o.requires
	$(MAKE) -f test/CMakeFiles/tsp_test.dir/build.make test/CMakeFiles/tsp_test.dir/tsp_test.cc.o.provides.build
.PHONY : test/CMakeFiles/tsp_test.dir/tsp_test.cc.o.provides

test/CMakeFiles/tsp_test.dir/tsp_test.cc.o.provides.build: test/CMakeFiles/tsp_test.dir/tsp_test.cc.o


# Object files for target tsp_test
tsp_test_OBJECTS = \
"CMakeFiles/tsp_test.dir/tsp_test.cc.o"

# External object files for target tsp_test
tsp_test_EXTERNAL_OBJECTS =

test/tsp_test: test/CMakeFiles/tsp_test.dir/tsp_test.cc.o
test/tsp_test: test/CMakeFiles/tsp_test.dir/build.make
test/tsp_test: lemon/libemon.a
test/tsp_test: /usr/local/lib/libglpk.dylib
test/tsp_test: test/CMakeFiles/tsp_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable tsp_test"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tsp_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/tsp_test.dir/build: test/tsp_test

.PHONY : test/CMakeFiles/tsp_test.dir/build

test/CMakeFiles/tsp_test.dir/requires: test/CMakeFiles/tsp_test.dir/tsp_test.cc.o.requires

.PHONY : test/CMakeFiles/tsp_test.dir/requires

test/CMakeFiles/tsp_test.dir/clean:
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/test && $(CMAKE_COMMAND) -P CMakeFiles/tsp_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/tsp_test.dir/clean

test/CMakeFiles/tsp_test.dir/depend:
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/alvis/PhD/fcla/lib/lemon-1.3.1 /Users/alvis/PhD/fcla/lib/lemon-1.3.1/test /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/test /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/test/CMakeFiles/tsp_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/tsp_test.dir/depend

