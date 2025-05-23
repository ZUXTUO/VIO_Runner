# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/olsc/VIO/colmap_localization/tool_map/fbow

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/olsc/VIO/colmap_localization/tool_map/fbow/build

# Include any dependencies generated for this target.
include src/CMakeFiles/fbow.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/fbow.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/fbow.dir/flags.make

src/CMakeFiles/fbow.dir/fbow.cpp.o: src/CMakeFiles/fbow.dir/flags.make
src/CMakeFiles/fbow.dir/fbow.cpp.o: ../src/fbow.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/olsc/VIO/colmap_localization/tool_map/fbow/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/fbow.dir/fbow.cpp.o"
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/fbow.dir/fbow.cpp.o -c /home/olsc/VIO/colmap_localization/tool_map/fbow/src/fbow.cpp

src/CMakeFiles/fbow.dir/fbow.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/fbow.dir/fbow.cpp.i"
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/olsc/VIO/colmap_localization/tool_map/fbow/src/fbow.cpp > CMakeFiles/fbow.dir/fbow.cpp.i

src/CMakeFiles/fbow.dir/fbow.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/fbow.dir/fbow.cpp.s"
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/olsc/VIO/colmap_localization/tool_map/fbow/src/fbow.cpp -o CMakeFiles/fbow.dir/fbow.cpp.s

src/CMakeFiles/fbow.dir/vocabulary_creator.cpp.o: src/CMakeFiles/fbow.dir/flags.make
src/CMakeFiles/fbow.dir/vocabulary_creator.cpp.o: ../src/vocabulary_creator.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/olsc/VIO/colmap_localization/tool_map/fbow/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/fbow.dir/vocabulary_creator.cpp.o"
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/fbow.dir/vocabulary_creator.cpp.o -c /home/olsc/VIO/colmap_localization/tool_map/fbow/src/vocabulary_creator.cpp

src/CMakeFiles/fbow.dir/vocabulary_creator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/fbow.dir/vocabulary_creator.cpp.i"
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/olsc/VIO/colmap_localization/tool_map/fbow/src/vocabulary_creator.cpp > CMakeFiles/fbow.dir/vocabulary_creator.cpp.i

src/CMakeFiles/fbow.dir/vocabulary_creator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/fbow.dir/vocabulary_creator.cpp.s"
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/olsc/VIO/colmap_localization/tool_map/fbow/src/vocabulary_creator.cpp -o CMakeFiles/fbow.dir/vocabulary_creator.cpp.s

# Object files for target fbow
fbow_OBJECTS = \
"CMakeFiles/fbow.dir/fbow.cpp.o" \
"CMakeFiles/fbow.dir/vocabulary_creator.cpp.o"

# External object files for target fbow
fbow_EXTERNAL_OBJECTS =

src/libfbow.so.0.0.1: src/CMakeFiles/fbow.dir/fbow.cpp.o
src/libfbow.so.0.0.1: src/CMakeFiles/fbow.dir/vocabulary_creator.cpp.o
src/libfbow.so.0.0.1: src/CMakeFiles/fbow.dir/build.make
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_stitching.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_superres.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_videostab.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_aruco.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_bgsegm.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_bioinspired.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_ccalib.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_dnn_objdetect.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_dpm.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_face.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_freetype.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_fuzzy.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_hfs.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_img_hash.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_line_descriptor.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_optflow.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_reg.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_rgbd.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_saliency.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_stereo.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_structured_light.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_surface_matching.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_tracking.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_xfeatures2d.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_ximgproc.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_xobjdetect.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_xphoto.so.3.4.20
src/libfbow.so.0.0.1: /usr/lib/gcc/x86_64-linux-gnu/9/libgomp.so
src/libfbow.so.0.0.1: /usr/lib/x86_64-linux-gnu/libpthread.so
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_shape.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_highgui.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_videoio.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_phase_unwrapping.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_video.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_datasets.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_plot.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_text.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_dnn.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_ml.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_imgcodecs.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_objdetect.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_calib3d.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_features2d.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_flann.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_photo.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_imgproc.so.3.4.20
src/libfbow.so.0.0.1: /usr/local/lib/libopencv_core.so.3.4.20
src/libfbow.so.0.0.1: src/CMakeFiles/fbow.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/olsc/VIO/colmap_localization/tool_map/fbow/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX shared library libfbow.so"
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/fbow.dir/link.txt --verbose=$(VERBOSE)
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src && $(CMAKE_COMMAND) -E cmake_symlink_library libfbow.so.0.0.1 libfbow.so.0.0 libfbow.so

src/libfbow.so.0.0: src/libfbow.so.0.0.1
	@$(CMAKE_COMMAND) -E touch_nocreate src/libfbow.so.0.0

src/libfbow.so: src/libfbow.so.0.0.1
	@$(CMAKE_COMMAND) -E touch_nocreate src/libfbow.so

# Rule to build all files generated by this target.
src/CMakeFiles/fbow.dir/build: src/libfbow.so

.PHONY : src/CMakeFiles/fbow.dir/build

src/CMakeFiles/fbow.dir/clean:
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src && $(CMAKE_COMMAND) -P CMakeFiles/fbow.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/fbow.dir/clean

src/CMakeFiles/fbow.dir/depend:
	cd /home/olsc/VIO/colmap_localization/tool_map/fbow/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/olsc/VIO/colmap_localization/tool_map/fbow /home/olsc/VIO/colmap_localization/tool_map/fbow/src /home/olsc/VIO/colmap_localization/tool_map/fbow/build /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src /home/olsc/VIO/colmap_localization/tool_map/fbow/build/src/CMakeFiles/fbow.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/fbow.dir/depend

