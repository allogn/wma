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
include lemon/CMakeFiles/lemon.dir/depend.make

# Include the progress variables for this target.
include lemon/CMakeFiles/lemon.dir/progress.make

# Include the compile flags for this target's objects.
include lemon/CMakeFiles/lemon.dir/flags.make

lemon/CMakeFiles/lemon.dir/arg_parser.cc.o: lemon/CMakeFiles/lemon.dir/flags.make
lemon/CMakeFiles/lemon.dir/arg_parser.cc.o: ../lemon/arg_parser.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lemon/CMakeFiles/lemon.dir/arg_parser.cc.o"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lemon.dir/arg_parser.cc.o -c /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/arg_parser.cc

lemon/CMakeFiles/lemon.dir/arg_parser.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lemon.dir/arg_parser.cc.i"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/arg_parser.cc > CMakeFiles/lemon.dir/arg_parser.cc.i

lemon/CMakeFiles/lemon.dir/arg_parser.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lemon.dir/arg_parser.cc.s"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/arg_parser.cc -o CMakeFiles/lemon.dir/arg_parser.cc.s

lemon/CMakeFiles/lemon.dir/arg_parser.cc.o.requires:

.PHONY : lemon/CMakeFiles/lemon.dir/arg_parser.cc.o.requires

lemon/CMakeFiles/lemon.dir/arg_parser.cc.o.provides: lemon/CMakeFiles/lemon.dir/arg_parser.cc.o.requires
	$(MAKE) -f lemon/CMakeFiles/lemon.dir/build.make lemon/CMakeFiles/lemon.dir/arg_parser.cc.o.provides.build
.PHONY : lemon/CMakeFiles/lemon.dir/arg_parser.cc.o.provides

lemon/CMakeFiles/lemon.dir/arg_parser.cc.o.provides.build: lemon/CMakeFiles/lemon.dir/arg_parser.cc.o


lemon/CMakeFiles/lemon.dir/base.cc.o: lemon/CMakeFiles/lemon.dir/flags.make
lemon/CMakeFiles/lemon.dir/base.cc.o: ../lemon/base.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object lemon/CMakeFiles/lemon.dir/base.cc.o"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lemon.dir/base.cc.o -c /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/base.cc

lemon/CMakeFiles/lemon.dir/base.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lemon.dir/base.cc.i"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/base.cc > CMakeFiles/lemon.dir/base.cc.i

lemon/CMakeFiles/lemon.dir/base.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lemon.dir/base.cc.s"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/base.cc -o CMakeFiles/lemon.dir/base.cc.s

lemon/CMakeFiles/lemon.dir/base.cc.o.requires:

.PHONY : lemon/CMakeFiles/lemon.dir/base.cc.o.requires

lemon/CMakeFiles/lemon.dir/base.cc.o.provides: lemon/CMakeFiles/lemon.dir/base.cc.o.requires
	$(MAKE) -f lemon/CMakeFiles/lemon.dir/build.make lemon/CMakeFiles/lemon.dir/base.cc.o.provides.build
.PHONY : lemon/CMakeFiles/lemon.dir/base.cc.o.provides

lemon/CMakeFiles/lemon.dir/base.cc.o.provides.build: lemon/CMakeFiles/lemon.dir/base.cc.o


lemon/CMakeFiles/lemon.dir/color.cc.o: lemon/CMakeFiles/lemon.dir/flags.make
lemon/CMakeFiles/lemon.dir/color.cc.o: ../lemon/color.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object lemon/CMakeFiles/lemon.dir/color.cc.o"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lemon.dir/color.cc.o -c /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/color.cc

lemon/CMakeFiles/lemon.dir/color.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lemon.dir/color.cc.i"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/color.cc > CMakeFiles/lemon.dir/color.cc.i

lemon/CMakeFiles/lemon.dir/color.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lemon.dir/color.cc.s"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/color.cc -o CMakeFiles/lemon.dir/color.cc.s

lemon/CMakeFiles/lemon.dir/color.cc.o.requires:

.PHONY : lemon/CMakeFiles/lemon.dir/color.cc.o.requires

lemon/CMakeFiles/lemon.dir/color.cc.o.provides: lemon/CMakeFiles/lemon.dir/color.cc.o.requires
	$(MAKE) -f lemon/CMakeFiles/lemon.dir/build.make lemon/CMakeFiles/lemon.dir/color.cc.o.provides.build
.PHONY : lemon/CMakeFiles/lemon.dir/color.cc.o.provides

lemon/CMakeFiles/lemon.dir/color.cc.o.provides.build: lemon/CMakeFiles/lemon.dir/color.cc.o


lemon/CMakeFiles/lemon.dir/lp_base.cc.o: lemon/CMakeFiles/lemon.dir/flags.make
lemon/CMakeFiles/lemon.dir/lp_base.cc.o: ../lemon/lp_base.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object lemon/CMakeFiles/lemon.dir/lp_base.cc.o"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lemon.dir/lp_base.cc.o -c /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/lp_base.cc

lemon/CMakeFiles/lemon.dir/lp_base.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lemon.dir/lp_base.cc.i"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/lp_base.cc > CMakeFiles/lemon.dir/lp_base.cc.i

lemon/CMakeFiles/lemon.dir/lp_base.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lemon.dir/lp_base.cc.s"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/lp_base.cc -o CMakeFiles/lemon.dir/lp_base.cc.s

lemon/CMakeFiles/lemon.dir/lp_base.cc.o.requires:

.PHONY : lemon/CMakeFiles/lemon.dir/lp_base.cc.o.requires

lemon/CMakeFiles/lemon.dir/lp_base.cc.o.provides: lemon/CMakeFiles/lemon.dir/lp_base.cc.o.requires
	$(MAKE) -f lemon/CMakeFiles/lemon.dir/build.make lemon/CMakeFiles/lemon.dir/lp_base.cc.o.provides.build
.PHONY : lemon/CMakeFiles/lemon.dir/lp_base.cc.o.provides

lemon/CMakeFiles/lemon.dir/lp_base.cc.o.provides.build: lemon/CMakeFiles/lemon.dir/lp_base.cc.o


lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o: lemon/CMakeFiles/lemon.dir/flags.make
lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o: ../lemon/lp_skeleton.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lemon.dir/lp_skeleton.cc.o -c /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/lp_skeleton.cc

lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lemon.dir/lp_skeleton.cc.i"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/lp_skeleton.cc > CMakeFiles/lemon.dir/lp_skeleton.cc.i

lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lemon.dir/lp_skeleton.cc.s"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/lp_skeleton.cc -o CMakeFiles/lemon.dir/lp_skeleton.cc.s

lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o.requires:

.PHONY : lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o.requires

lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o.provides: lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o.requires
	$(MAKE) -f lemon/CMakeFiles/lemon.dir/build.make lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o.provides.build
.PHONY : lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o.provides

lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o.provides.build: lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o


lemon/CMakeFiles/lemon.dir/random.cc.o: lemon/CMakeFiles/lemon.dir/flags.make
lemon/CMakeFiles/lemon.dir/random.cc.o: ../lemon/random.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object lemon/CMakeFiles/lemon.dir/random.cc.o"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lemon.dir/random.cc.o -c /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/random.cc

lemon/CMakeFiles/lemon.dir/random.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lemon.dir/random.cc.i"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/random.cc > CMakeFiles/lemon.dir/random.cc.i

lemon/CMakeFiles/lemon.dir/random.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lemon.dir/random.cc.s"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/random.cc -o CMakeFiles/lemon.dir/random.cc.s

lemon/CMakeFiles/lemon.dir/random.cc.o.requires:

.PHONY : lemon/CMakeFiles/lemon.dir/random.cc.o.requires

lemon/CMakeFiles/lemon.dir/random.cc.o.provides: lemon/CMakeFiles/lemon.dir/random.cc.o.requires
	$(MAKE) -f lemon/CMakeFiles/lemon.dir/build.make lemon/CMakeFiles/lemon.dir/random.cc.o.provides.build
.PHONY : lemon/CMakeFiles/lemon.dir/random.cc.o.provides

lemon/CMakeFiles/lemon.dir/random.cc.o.provides.build: lemon/CMakeFiles/lemon.dir/random.cc.o


lemon/CMakeFiles/lemon.dir/bits/windows.cc.o: lemon/CMakeFiles/lemon.dir/flags.make
lemon/CMakeFiles/lemon.dir/bits/windows.cc.o: ../lemon/bits/windows.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object lemon/CMakeFiles/lemon.dir/bits/windows.cc.o"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lemon.dir/bits/windows.cc.o -c /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/bits/windows.cc

lemon/CMakeFiles/lemon.dir/bits/windows.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lemon.dir/bits/windows.cc.i"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/bits/windows.cc > CMakeFiles/lemon.dir/bits/windows.cc.i

lemon/CMakeFiles/lemon.dir/bits/windows.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lemon.dir/bits/windows.cc.s"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/bits/windows.cc -o CMakeFiles/lemon.dir/bits/windows.cc.s

lemon/CMakeFiles/lemon.dir/bits/windows.cc.o.requires:

.PHONY : lemon/CMakeFiles/lemon.dir/bits/windows.cc.o.requires

lemon/CMakeFiles/lemon.dir/bits/windows.cc.o.provides: lemon/CMakeFiles/lemon.dir/bits/windows.cc.o.requires
	$(MAKE) -f lemon/CMakeFiles/lemon.dir/build.make lemon/CMakeFiles/lemon.dir/bits/windows.cc.o.provides.build
.PHONY : lemon/CMakeFiles/lemon.dir/bits/windows.cc.o.provides

lemon/CMakeFiles/lemon.dir/bits/windows.cc.o.provides.build: lemon/CMakeFiles/lemon.dir/bits/windows.cc.o


lemon/CMakeFiles/lemon.dir/glpk.cc.o: lemon/CMakeFiles/lemon.dir/flags.make
lemon/CMakeFiles/lemon.dir/glpk.cc.o: ../lemon/glpk.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object lemon/CMakeFiles/lemon.dir/glpk.cc.o"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lemon.dir/glpk.cc.o -c /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/glpk.cc

lemon/CMakeFiles/lemon.dir/glpk.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lemon.dir/glpk.cc.i"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/glpk.cc > CMakeFiles/lemon.dir/glpk.cc.i

lemon/CMakeFiles/lemon.dir/glpk.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lemon.dir/glpk.cc.s"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon/glpk.cc -o CMakeFiles/lemon.dir/glpk.cc.s

lemon/CMakeFiles/lemon.dir/glpk.cc.o.requires:

.PHONY : lemon/CMakeFiles/lemon.dir/glpk.cc.o.requires

lemon/CMakeFiles/lemon.dir/glpk.cc.o.provides: lemon/CMakeFiles/lemon.dir/glpk.cc.o.requires
	$(MAKE) -f lemon/CMakeFiles/lemon.dir/build.make lemon/CMakeFiles/lemon.dir/glpk.cc.o.provides.build
.PHONY : lemon/CMakeFiles/lemon.dir/glpk.cc.o.provides

lemon/CMakeFiles/lemon.dir/glpk.cc.o.provides.build: lemon/CMakeFiles/lemon.dir/glpk.cc.o


# Object files for target lemon
lemon_OBJECTS = \
"CMakeFiles/lemon.dir/arg_parser.cc.o" \
"CMakeFiles/lemon.dir/base.cc.o" \
"CMakeFiles/lemon.dir/color.cc.o" \
"CMakeFiles/lemon.dir/lp_base.cc.o" \
"CMakeFiles/lemon.dir/lp_skeleton.cc.o" \
"CMakeFiles/lemon.dir/random.cc.o" \
"CMakeFiles/lemon.dir/bits/windows.cc.o" \
"CMakeFiles/lemon.dir/glpk.cc.o"

# External object files for target lemon
lemon_EXTERNAL_OBJECTS =

lemon/libemon.a: lemon/CMakeFiles/lemon.dir/arg_parser.cc.o
lemon/libemon.a: lemon/CMakeFiles/lemon.dir/base.cc.o
lemon/libemon.a: lemon/CMakeFiles/lemon.dir/color.cc.o
lemon/libemon.a: lemon/CMakeFiles/lemon.dir/lp_base.cc.o
lemon/libemon.a: lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o
lemon/libemon.a: lemon/CMakeFiles/lemon.dir/random.cc.o
lemon/libemon.a: lemon/CMakeFiles/lemon.dir/bits/windows.cc.o
lemon/libemon.a: lemon/CMakeFiles/lemon.dir/glpk.cc.o
lemon/libemon.a: lemon/CMakeFiles/lemon.dir/build.make
lemon/libemon.a: lemon/CMakeFiles/lemon.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX static library libemon.a"
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && $(CMAKE_COMMAND) -P CMakeFiles/lemon.dir/cmake_clean_target.cmake
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lemon.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lemon/CMakeFiles/lemon.dir/build: lemon/libemon.a

.PHONY : lemon/CMakeFiles/lemon.dir/build

lemon/CMakeFiles/lemon.dir/requires: lemon/CMakeFiles/lemon.dir/arg_parser.cc.o.requires
lemon/CMakeFiles/lemon.dir/requires: lemon/CMakeFiles/lemon.dir/base.cc.o.requires
lemon/CMakeFiles/lemon.dir/requires: lemon/CMakeFiles/lemon.dir/color.cc.o.requires
lemon/CMakeFiles/lemon.dir/requires: lemon/CMakeFiles/lemon.dir/lp_base.cc.o.requires
lemon/CMakeFiles/lemon.dir/requires: lemon/CMakeFiles/lemon.dir/lp_skeleton.cc.o.requires
lemon/CMakeFiles/lemon.dir/requires: lemon/CMakeFiles/lemon.dir/random.cc.o.requires
lemon/CMakeFiles/lemon.dir/requires: lemon/CMakeFiles/lemon.dir/bits/windows.cc.o.requires
lemon/CMakeFiles/lemon.dir/requires: lemon/CMakeFiles/lemon.dir/glpk.cc.o.requires

.PHONY : lemon/CMakeFiles/lemon.dir/requires

lemon/CMakeFiles/lemon.dir/clean:
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon && $(CMAKE_COMMAND) -P CMakeFiles/lemon.dir/cmake_clean.cmake
.PHONY : lemon/CMakeFiles/lemon.dir/clean

lemon/CMakeFiles/lemon.dir/depend:
	cd /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/alvis/PhD/fcla/lib/lemon-1.3.1 /Users/alvis/PhD/fcla/lib/lemon-1.3.1/lemon /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon /Users/alvis/PhD/fcla/lib/lemon-1.3.1/build/lemon/CMakeFiles/lemon.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lemon/CMakeFiles/lemon.dir/depend

