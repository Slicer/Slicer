project(BRAINSResample)
set(LOCAL_PROJECT_NAME BRAINSResample)
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


# No need to add with nothing to compile. add_subdirectory(BRAINSResample_Common)
include(${BRAINS_CMAKE_HELPER_DIR}/IJMacros.txt)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/BRAINSResampleCommonLib.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/BRAINSResampleCommonLib.h
  )

###
configure_file(${BRAINSResample_SOURCE_DIR}/CTestCustom.ctest ${BRAINSResample_BINARY_DIR}/CTestCustom.ctest COPYONLY)

include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBRAINS3BuildMacros.cmake)
###


set(CLP BRAINSResample)
#set(${CLP}_SOURCE BRAINSResample.cxx ${BRAINSResample_SOURCE_DIR}/BRAINSResamplePrimary.cxx)
set(${CLP}_SOURCE BRAINSResample.cxx )
set(${CLP}_PRIMARY_SOURCE ${BRAINSResample_SOURCE_DIR}/BRAINSResamplePrimary.cxx)
#CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSResamplePrimary.xml "${${CLP}_SOURCE}" "")
CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSResamplePrimary.xml "${${CLP}_SOURCE}" "${${CLP}_PRIMARY_SOURCE}" "BRAINSResamplePrimary" "")


if(BRAINS_BUILD) ## ONLY BUILD FOR BACKWARDS COMPATIBILITY IN BRAINS ENVIRONMENT
  set(${CLP}_SOURCE BRAINSResample.cxx )
  set(${CLP}_PRIMARY_SOURCE ${BRAINSResample_SOURCE_DIR}/BRAINSResamplePrimary.cxx)
  CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSResamplePrimary.xml "${${CLP}_SOURCE}" "${${CLP}_PRIMARY_SOURCE}" "BRAINSResamplePrimary" "")
## HACK -- Need to write other backwards compatibility wrappers for the old names of BRAINSResampleCLP
endif(BRAINS_BUILD) ## ONLY BUILD FOR BACKWARDS COMPATIBILITY IN BRAINS ENVIRONMENT

### Add the Testing Subdirectory.
if(BUILD_TESTING)
  add_subdirectory(TestSuite)
endif(BUILD_TESTING)
