# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/peaceless/code/webServer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/peaceless/code/webServer

# Include any dependencies generated for this target.
include CMakeFiles/WebServer.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/WebServer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/WebServer.dir/flags.make

CMakeFiles/WebServer.dir/main/main.cpp.o: CMakeFiles/WebServer.dir/flags.make
CMakeFiles/WebServer.dir/main/main.cpp.o: main/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/peaceless/code/webServer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/WebServer.dir/main/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/WebServer.dir/main/main.cpp.o -c /home/peaceless/code/webServer/main/main.cpp

CMakeFiles/WebServer.dir/main/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/WebServer.dir/main/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/peaceless/code/webServer/main/main.cpp > CMakeFiles/WebServer.dir/main/main.cpp.i

CMakeFiles/WebServer.dir/main/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/WebServer.dir/main/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/peaceless/code/webServer/main/main.cpp -o CMakeFiles/WebServer.dir/main/main.cpp.s

CMakeFiles/WebServer.dir/main/main.cpp.o.requires:

.PHONY : CMakeFiles/WebServer.dir/main/main.cpp.o.requires

CMakeFiles/WebServer.dir/main/main.cpp.o.provides: CMakeFiles/WebServer.dir/main/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/WebServer.dir/build.make CMakeFiles/WebServer.dir/main/main.cpp.o.provides.build
.PHONY : CMakeFiles/WebServer.dir/main/main.cpp.o.provides

CMakeFiles/WebServer.dir/main/main.cpp.o.provides.build: CMakeFiles/WebServer.dir/main/main.cpp.o


# Object files for target WebServer
WebServer_OBJECTS = \
"CMakeFiles/WebServer.dir/main/main.cpp.o"

# External object files for target WebServer
WebServer_EXTERNAL_OBJECTS =

build/WebServer: CMakeFiles/WebServer.dir/main/main.cpp.o
build/WebServer: CMakeFiles/WebServer.dir/build.make
build/WebServer: lib/liblib.so
build/WebServer: CMakeFiles/WebServer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/peaceless/code/webServer/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable build/WebServer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/WebServer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/WebServer.dir/build: build/WebServer

.PHONY : CMakeFiles/WebServer.dir/build

CMakeFiles/WebServer.dir/requires: CMakeFiles/WebServer.dir/main/main.cpp.o.requires

.PHONY : CMakeFiles/WebServer.dir/requires

CMakeFiles/WebServer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/WebServer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/WebServer.dir/clean

CMakeFiles/WebServer.dir/depend:
	cd /home/peaceless/code/webServer && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/peaceless/code/webServer /home/peaceless/code/webServer /home/peaceless/code/webServer /home/peaceless/code/webServer /home/peaceless/code/webServer/CMakeFiles/WebServer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/WebServer.dir/depend

