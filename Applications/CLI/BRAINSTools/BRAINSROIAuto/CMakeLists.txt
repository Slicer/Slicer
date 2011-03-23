project(BRAINSROIAuto)
set(LOCAL_PROJECT_NAME BRAINSROIAuto)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

enable_testing()
include(Dart)
include(CPack)
include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBuildMacros.cmake)
include(${BRAINS_CMAKE_HELPER_DIR}/PreventInSourceBuilds.cmake)

###
SETIFEMPTY(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
SETIFEMPTY(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
SETIFEMPTY(CMAKE_BUNDLE_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/bin)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

SETIFEMPTY(MIDAS_REST_URL "http://midas.kitware.com/api/rest" CACHE STRING "The MIDAS server where testing data resides")

###
CHECKIFSLICER3BUILD()  ## Call the convenience macro

if(NOT ITK_FOUND)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
endif(NOT ITK_FOUND)

find_package(BRAINSCommonLib NO_MODULE REQUIRED)
include(${BRAINSCommonLib_USE_FILE})

#-----------------------------------------------------------------------------
# Output directories.
#
include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBRAINS3BuildMacros.cmake)
#SETOPTIONALDEBUGIMAGEVIEWER()


# No need to add with nothing to compile. add_subdirectory(BRAINSROIAuto_Common)
include(${BRAINS_CMAKE_HELPER_DIR}/IJMacros.txt)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/BRAINSROIAutoCommonLib.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/BRAINSROIAutoCommonLib.h
  )


set(CLP BRAINSROIAuto)
set(${CLP}_SOURCE BRAINSROIAuto.cxx )
set(${CLP}_PRIMARY_SOURCE BRAINSROIAuto.cxx)
CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSROIAuto.xml "${${CLP}_SOURCE}" "${${CLP}_PRIMARY_SOURCE}" "main" "")
target_link_libraries( ${CLP} BRAINSCommonLib ${ITK_LIBRARIES} ${OPTIONAL_DEBUG_LINK_LIBRARIES})

if(BUILD_TESTING)
    add_subdirectory(TestSuite)
endif(BUILD_TESTING)
