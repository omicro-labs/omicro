# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build

# Include any dependencies generated for this target.
include CMakeFiles/lib_g2logger_shared.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lib_g2logger_shared.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lib_g2logger_shared.dir/flags.make

CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.o: CMakeFiles/lib_g2logger_shared.dir/flags.make
CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.o: ../src/active.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.o -c /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/active.cpp

CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/active.cpp > CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.i

CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/active.cpp -o CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.s

CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.o: CMakeFiles/lib_g2logger_shared.dir/flags.make
CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.o: ../src/crashhandler_unix.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.o -c /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/crashhandler_unix.cpp

CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/crashhandler_unix.cpp > CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.i

CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/crashhandler_unix.cpp -o CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.s

CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.o: CMakeFiles/lib_g2logger_shared.dir/flags.make
CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.o: ../src/g2log.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.o -c /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/g2log.cpp

CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/g2log.cpp > CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.i

CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/g2log.cpp -o CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.s

CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.o: CMakeFiles/lib_g2logger_shared.dir/flags.make
CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.o: ../src/g2logworker.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.o -c /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/g2logworker.cpp

CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/g2logworker.cpp > CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.i

CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/g2logworker.cpp -o CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.s

CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.o: CMakeFiles/lib_g2logger_shared.dir/flags.make
CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.o: ../src/g2time.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.o -c /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/g2time.cpp

CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/g2time.cpp > CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.i

CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/src/g2time.cpp -o CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.s

# Object files for target lib_g2logger_shared
lib_g2logger_shared_OBJECTS = \
"CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.o" \
"CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.o" \
"CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.o" \
"CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.o" \
"CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.o"

# External object files for target lib_g2logger_shared
lib_g2logger_shared_EXTERNAL_OBJECTS =

liblib_g2logger_shared.so: CMakeFiles/lib_g2logger_shared.dir/src/active.cpp.o
liblib_g2logger_shared.so: CMakeFiles/lib_g2logger_shared.dir/src/crashhandler_unix.cpp.o
liblib_g2logger_shared.so: CMakeFiles/lib_g2logger_shared.dir/src/g2log.cpp.o
liblib_g2logger_shared.so: CMakeFiles/lib_g2logger_shared.dir/src/g2logworker.cpp.o
liblib_g2logger_shared.so: CMakeFiles/lib_g2logger_shared.dir/src/g2time.cpp.o
liblib_g2logger_shared.so: CMakeFiles/lib_g2logger_shared.dir/build.make
liblib_g2logger_shared.so: CMakeFiles/lib_g2logger_shared.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX shared library liblib_g2logger_shared.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lib_g2logger_shared.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lib_g2logger_shared.dir/build: liblib_g2logger_shared.so

.PHONY : CMakeFiles/lib_g2logger_shared.dir/build

CMakeFiles/lib_g2logger_shared.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lib_g2logger_shared.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lib_g2logger_shared.dir/clean

CMakeFiles/lib_g2logger_shared.dir/depend:
	cd /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build /home/dev2/github_omicro/omicro/src/Web/kcpasio/asio_kcp/third_party/g2log/build/CMakeFiles/lib_g2logger_shared.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lib_g2logger_shared.dir/depend

