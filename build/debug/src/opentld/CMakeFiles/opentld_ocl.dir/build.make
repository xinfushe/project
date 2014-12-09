# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_SOURCE_DIR = /home/slilylsu/Desktop/project-repo/OpenTLD

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/slilylsu/Desktop/project-repo/build/debug

# Include any dependencies generated for this target.
include src/opentld/CMakeFiles/opentld_ocl.dir/depend.make

# Include the progress variables for this target.
include src/opentld/CMakeFiles/opentld_ocl.dir/progress.make

# Include the compile flags for this target's objects.
include src/opentld/CMakeFiles/opentld_ocl.dir/flags.make

src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o: src/opentld/CMakeFiles/opentld_ocl.dir/flags.make
src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o: /home/slilylsu/Desktop/project-repo/OpenTLD/src/opentld/OpenTLD.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/slilylsu/Desktop/project-repo/build/debug/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o"
	cd /home/slilylsu/Desktop/project-repo/build/debug/src/opentld && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o -c /home/slilylsu/Desktop/project-repo/OpenTLD/src/opentld/OpenTLD.cpp

src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.i"
	cd /home/slilylsu/Desktop/project-repo/build/debug/src/opentld && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/slilylsu/Desktop/project-repo/OpenTLD/src/opentld/OpenTLD.cpp > CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.i

src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.s"
	cd /home/slilylsu/Desktop/project-repo/build/debug/src/opentld && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/slilylsu/Desktop/project-repo/OpenTLD/src/opentld/OpenTLD.cpp -o CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.s

src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o.requires:
.PHONY : src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o.requires

src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o.provides: src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o.requires
	$(MAKE) -f src/opentld/CMakeFiles/opentld_ocl.dir/build.make src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o.provides.build
.PHONY : src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o.provides

src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o.provides.build: src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o

# Object files for target opentld_ocl
opentld_ocl_OBJECTS = \
"CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o"

# External object files for target opentld_ocl
opentld_ocl_EXTERNAL_OBJECTS =

bin/opentld_ocl: src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o
bin/opentld_ocl: src/opentld/CMakeFiles/opentld_ocl.dir/build.make
bin/opentld_ocl: lib/libmain.a
bin/opentld_ocl: lib/libopentld.a
bin/opentld_ocl: lib/libcvblobs.a
bin/opentld_ocl: lib/libconfig++.a
bin/opentld_ocl: /usr/local/lib/libopencv_videostab.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_video.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_ts.a
bin/opentld_ocl: /usr/local/lib/libopencv_superres.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_stitching.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_photo.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_ocl.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_objdetect.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_nonfree.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_ml.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_legacy.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_imgproc.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_highgui.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_gpu.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_flann.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_features2d.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_core.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_contrib.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_calib3d.so.2.4.10
bin/opentld_ocl: /usr/local/lib/beignet/libcl.so
bin/opentld_ocl: /usr/local/lib/libopencv_nonfree.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_ocl.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_gpu.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_photo.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_objdetect.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_legacy.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_video.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_ml.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_calib3d.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_features2d.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_highgui.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_imgproc.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_flann.so.2.4.10
bin/opentld_ocl: /usr/local/lib/libopencv_core.so.2.4.10
bin/opentld_ocl: src/opentld/CMakeFiles/opentld_ocl.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ../../bin/opentld_ocl"
	cd /home/slilylsu/Desktop/project-repo/build/debug/src/opentld && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/opentld_ocl.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/opentld/CMakeFiles/opentld_ocl.dir/build: bin/opentld_ocl
.PHONY : src/opentld/CMakeFiles/opentld_ocl.dir/build

src/opentld/CMakeFiles/opentld_ocl.dir/requires: src/opentld/CMakeFiles/opentld_ocl.dir/OpenTLD.cpp.o.requires
.PHONY : src/opentld/CMakeFiles/opentld_ocl.dir/requires

src/opentld/CMakeFiles/opentld_ocl.dir/clean:
	cd /home/slilylsu/Desktop/project-repo/build/debug/src/opentld && $(CMAKE_COMMAND) -P CMakeFiles/opentld_ocl.dir/cmake_clean.cmake
.PHONY : src/opentld/CMakeFiles/opentld_ocl.dir/clean

src/opentld/CMakeFiles/opentld_ocl.dir/depend:
	cd /home/slilylsu/Desktop/project-repo/build/debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/slilylsu/Desktop/project-repo/OpenTLD /home/slilylsu/Desktop/project-repo/OpenTLD/src/opentld /home/slilylsu/Desktop/project-repo/build/debug /home/slilylsu/Desktop/project-repo/build/debug/src/opentld /home/slilylsu/Desktop/project-repo/build/debug/src/opentld/CMakeFiles/opentld_ocl.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/opentld/CMakeFiles/opentld_ocl.dir/depend

