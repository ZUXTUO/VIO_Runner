cmake_minimum_required(VERSION 3.10)
project(ulocalization)

set(FOLDER_NAME "tools")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find dependencies
find_package(OpenCV 3.4 QUIET)
if(NOT OpenCV_FOUND)
  find_package(OpenCV 2.4.3 QUIET)
  if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV > 2.4.3 not found.")
  endif()
endif()

set(CUDA_MIN_VERSION "7.0")
if(CUDA_ENABLED)
  find_package(CUDA ${CUDA_MIN_VERSION} QUIET)
endif()

find_package(PkgConfig REQUIRED)
find_package(Pangolin REQUIRED)
find_package(JNI REQUIRED)
if(NOT JNI_FOUND)
  message(FATAL_ERROR "JNI not found.")
endif()
find_package(COLMAP REQUIRED)
find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
  message("OPENMP FOUND")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# Include directories
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/JNIs/
  ${PROJECT_SOURCE_DIR}/src/
  ${PROJECT_SOURCE_DIR}/visual_index/
  ${JNI_INCLUDE_DIRS}
  ${COLMAP_INCLUDE_DIRS}
  ${Pangolin_INCLUDE_DIRS}
)
link_directories(${COLMAP_LINK_DIRS})

# RPATH configuration for bundled libraries
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/ulocalization_deps")
install(CODE [[
  file(MAKE_DIRECTORY "${INSTALL_LIB_DIR}")
  file(GLOB DEP_LIBS
    "/usr/lib/x86_64-linux-gnu/libopencv_core.so.*"
    "/usr/lib/x86_64-linux-gnu/libcolmap.so.*"
    "/usr/lib/x86_64-linux-gnu/libpangolin.so.*"
    "/usr/lib/x86_64-linux-gnu/libglog.so.*"
    "/usr/lib/x86_64-linux-gnu/libgflags.so.*"
    "/usr/lib/x86_64-linux-gnu/libomp.so.*"
  )
  foreach(_lib ${DEP_LIBS})
    file(INSTALL DESTINATION "${INSTALL_LIB_DIR}" TYPE SHARED_LIBRARY FILES "${_lib}")
  endforeach()
]])
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib/ulocalization_deps")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# Library target
add_library(ulocalization SHARED
  src/test_utils.cc
  src/UlocalizationPlugins.cc
  src/UlocalizationPluginsBeta.cc
  src/LocalizationLY.cc
  src/configuration.cc
  src/Viewer.cc
  src/MapSaver.cc
  src/MarkerFinder.cc
  JNIs/Java_com_utopa_arserver_localization_jni_LocalizationSDK.cpp
)
target_link_libraries(ulocalization
  ${OpenCV_LIBS}
  ${JNI_LIBRARIES}
  ${COLMAP_LIBRARIES}
  ${Pangolin_LIBRARIES}
)
set_target_properties(ulocalization PROPERTIES
  INSTALL_RPATH "$ORIGIN/../lib/ulocalization_deps"
)

# Only keep Runner executable
add_executable(Runner tests/Runner.cc)
target_link_libraries(Runner ulocalization)
set_target_properties(Runner PROPERTIES
  INSTALL_RPATH "$ORIGIN/../lib/ulocalization_deps"
)

# Add Client executable
add_executable(Client tests/Client.cc) # Add this line to create the Client executable
target_link_libraries(Client ulocalization) # Link Client to the ulocalization library
set_target_properties(Client PROPERTIES
  INSTALL_RPATH "$ORIGIN/../lib/ulocalization_deps"
)

# Install rules
install(TARGETS ulocalization Runner Client # Add Client to the list of targets to install
  EXPORT ulocalizationTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# Install headers
install(DIRECTORY src/
  DESTINATION include/ulocalization
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
install(DIRECTORY JNIs/
  DESTINATION include/ulocalization/JNIs
  FILES_MATCHING PATTERN "*.h"
)

# Export the package
install(EXPORT ulocalizationTargets
  FILE ulocalizationTargets.cmake
  NAMESPACE ulocalization::
  DESTINATION lib/cmake/ulocalization
)

# Package config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/ulocalizationConfigVersion.cmake"
  VERSION 1.0.0
  COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ulocalizationConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/ulocalizationConfig.cmake"
  INSTALL_DESTINATION lib/cmake/ulocalization
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/ulocalizationConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/ulocalizationConfigVersion.cmake"
  DESTINATION lib/cmake/ulocalization
)

# CPack configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "vio_ulocalization")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "3086003479@qq.com")
include(CPack)
