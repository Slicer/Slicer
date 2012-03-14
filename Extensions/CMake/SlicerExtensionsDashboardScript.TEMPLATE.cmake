#
# OS: Ubuntu 9.04  2.6.28-19-generic
# Hardware: x86_64 GNU/Linux
# GPU: NA

# Note: The specific version and processor type of this machine should be reported in the
# header above. Indeed, this file will be sent to the dashboard as a NOTE file.

cmake_minimum_required(VERSION 2.8.7)

#
# For additional information, see http://www.slicer.org/slicerWiki/index.php/Documentation/4.1/Developers/Tutorials/DashboardSetup
#

#
# Dashboard properties
#
set(MY_OPERATING_SYSTEM   "Linux") # Windows, Linux, Darwin...
set(MY_COMPILER           "g++4.4.3")
set(MY_QT_VERSION         "4.7")
set(Slicer_DIR            "$ENV{HOME}/Projects/Slicer4-Superbuild-Debug/Slicer-build")
set(CTEST_SITE            "karakoram.kitware") # for example: mymachine.kitware, mymachine.bwh.harvard.edu, ...
set(CTEST_DASHBOARD_ROOT  "$ENV{HOME}/Dashboards/")
set(CTEST_CMAKE_COMMAND   "$ENV{HOME}/Dashboards/Support/cmake-2.8.7-Linux-i386/bin/cmake")
# Open a shell and type in "cmake --help" to obtain the proper spelling of the generator
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(MY_BITNESS            "64")
set(MIDAS_PACKAGE_URL "slicer.kitware.com/midas3")
#set(MIDAS_PACKAGE_EMAIL "EMAIL")
#set(MIDAS_PACKAGE_API_KEY "API_KEY")

#
# Dashboard options
#
set(WITH_KWSTYLE FALSE)
set(WITH_MEMCHECK FALSE)
set(WITH_COVERAGE FALSE)
set(WITH_DOCUMENTATION FALSE)
#set(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY ) # for example: $ENV{HOME}/Projects/Doxygen
set(CTEST_BUILD_CONFIGURATION "Release")
set(CTEST_TEST_TIMEOUT 500)
set(CTEST_BUILD_FLAGS "") # Use multiple CPU cores to build. For example "-j4" on unix
set(CTEST_PARALLEL_LEVEL 8) # Number of tests running in parallel

find_program(CTEST_SVN_COMMAND NAMES svn)
find_program(CTEST_GIT_COMMAND NAMES git)
find_program(CTEST_COVERAGE_COMMAND NAMES gcov)
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
# experimental:
#     - run_ctest() macro will be called *ONE* time
#     - binary directory will *NOT* be cleaned
# continuous:
#     - run_ctest() macro will be called EVERY 5 minutes ...
#     - binary directory will *NOT* be cleaned
#     - configure/build will be executed *ONLY* if the repository has been updated
# nightly:
#     - run_ctest() macro will be called *ONE* time
#     - binary directory *WILL BE* cleaned
set(SCRIPT_MODE "experimental") # "experimental", "continuous", "nightly"

# You could invoke the script with the following syntax:
#  ctest -S karakoram_Slicer4_nightly.cmake -V


#
# Additional CMakeCache options
# For example:
#   ADDITIONAL_{C/CXX}_FLAGS: /MP -> Multi process build with MSVC
#
set(ADDITIONAL_CMAKECACHE_OPTION "
  ADDITIONAL_C_FLAGS:STRING=
  ADDITIONAL_CXX_FLAGS:STRING=
")

# Update the following variable to match the chosen build options. This variable is used to
# generate both the build directory and the build name.
set(BUILD_OPTIONS_STRING "${MY_BITNESS}bits-QT${MY_QT_VERSION}")

#
# Project specific properties
#
set(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/SlicerExtensions")
set(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/SlicerExtensions-build-${BUILD_OPTIONS_STRING}-${CTEST_BUILD_CONFIGURATION}-${SCRIPT_MODE}")

# set any extra environment variables here
if(UNIX)
  set(ENV{DISPLAY} ":0")
endif()


##########################################
# WARNING: DO NOT EDIT BEYOND THIS POINT #
##########################################

set(CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

#
# Project specific properties
#
set(CTEST_PROJECT_NAME "Slicer4")
set(CTEST_BUILD_NAME "${MY_OPERATING_SYSTEM}-${MY_COMPILER}-${BUILD_OPTIONS_STRING}-${CTEST_BUILD_CONFIGURATION}")

#
# Display build info
#
message("site name: ${CTEST_SITE}")
message("build name: ${CTEST_BUILD_NAME}")
message("script mode: ${SCRIPT_MODE}")
message("coverage: ${WITH_COVERAGE}, memcheck: ${WITH_MEMCHECK}")

#
# Convenient macro allowing to download a file
#
macro(downloadFile url dest)
  file(DOWNLOAD ${url} ${dest} STATUS status)
  list(GET status 0 error_code)
  list(GET status 1 error_msg)
  if(error_code)
    message(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
  endif()
endmacro()

#
# Download and include dashboard driver script
#
set(url http://svn.slicer.org/Slicer4/trunk/Extensions/CMake/SlicerExtensionsDashboardDriverScript.cmake)
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.driver)
downloadFile(${url} ${dest})
include(${dest})
