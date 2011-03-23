project(BRAINSDemonWarp)
set(LOCAL_PROJECT_NAME BRAINSDemonWarp)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

enable_testing()
include(Dart)
include(CPack)
include(${BRAINS_CMAKE_HELPER_DIR}/PreventInSourceBuilds.cmake)
include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBuildMacros.cmake)

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


# No need to add with nothing to compile. add_subdirectory(BRAINSFit_Common)
include(${BRAINS_CMAKE_HELPER_DIR}/IJMacros.txt)

###
configure_file(${BRAINSDemonWarp_SOURCE_DIR}/CTestCustom.ctest ${BRAINSDemonWarp_BINARY_DIR}/CTestCustom.ctest COPYONLY)

include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBRAINS3BuildMacros.cmake)
###

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/BRAINSDemonWarpCommonLib.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/BRAINSDemonWarpCommonLib.h
  )


set(BRAINSDemonWarpTemplatesLIB_SRCS
         BRAINSDemonWarpTemplates_Vdouble.cxx
         BRAINSDemonWarpTemplates_Vshort.cxx
         BRAINSDemonWarpTemplates_Vushort.cxx
         BRAINSDemonWarpTemplates_int.cxx
         BRAINSDemonWarpTemplates_uint.cxx
         BRAINSDemonWarpTemplates_Vfloat.cxx
         BRAINSDemonWarpTemplates_Vuchar.cxx
         BRAINSDemonWarpTemplates_double.cxx
         BRAINSDemonWarpTemplates_short.cxx
         BRAINSDemonWarpTemplates_ushort.cxx
         BRAINSDemonWarpTemplates_Vint.cxx
         BRAINSDemonWarpTemplates_Vuint.cxx
         BRAINSDemonWarpTemplates_float.cxx
         BRAINSDemonWarpTemplates_uchar.cxx
         BRAINSDemonWarpTemplates.cxx
)

set(CLP BRAINSDemonWarpCommonLib)
set(${CLP}_SOURCE ${BRAINSDemonWarpTemplatesLIB_SRCS})
## NOTE:  The BRAINSDemonWarpCommonLib includes BRAINSDemonWarpPrimary.cxx and must be configured against BRAINSDemonWarpPrimary.xml
CONFIGUREBRAINSORSLICERLIBRARY(${CLP} BRAINSDemonWarpPrimary.xml "${${CLP}_SOURCE}" "")
## NOTE:  The BRAINSDemonWarpCommonLib includes VBRAINSDemonWarpPrimary.cxx and must be configured against VBRAINSDemonWarpPrimary.xml

set(CLP BRAINSDemonWarp)
set(${CLP}_SOURCE BRAINSDemonWarp.cxx )
set(${CLP}_PRIMARY_SOURCE BRAINSDemonWarpPrimary.cxx)
CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSDemonWarpPrimary.xml "${${CLP}_SOURCE}" "${${CLP}_PRIMARY_SOURCE}" "BRAINSDemonWarpPrimary" "BRAINSDemonWarpCommonLib")

if(BRAINS_BUILD) ## ONLY BUILD FOR BACKWARDS COMPATIBILITY IN BRAINS ENVIRONMENT
  set(CLP BRAINSDemonWarpCLP) ## NOTE: This is completely redundant, but is needed to support all the old scripts that use the original name
  set(${CLP}_SOURCE BRAINSDemonWarp.cxx )
  set(${CLP}_PRIMARY_SOURCE BRAINSDemonWarpPrimary.cxx)
  CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSDemonWarpPrimary.xml "${${CLP}_SOURCE}" "${${CLP}_PRIMARY_SOURCE}" "BRAINSDemonWarpPrimary" "BRAINSDemonWarpCommonLib")
## HACK -- Need to write other backwards compatibility wrappers for the old names of BRAINSDemonWarpCLP
endif(BRAINS_BUILD) ## ONLY BUILD FOR BACKWARDS COMPATIBILITY IN BRAINS ENVIRONMENT

set(CLP VBRAINSDemonWarp)
set(${CLP}_SOURCE VBRAINSDemonWarp.cxx )
set(${CLP}_PRIMARY_SOURCE VBRAINSDemonWarpPrimary.cxx)
#CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} VBRAINSDemonWarpPrimary.xml "${${CLP}_SOURCE}" "BRAINSDemonWarpCommonLib")
CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} VBRAINSDemonWarpPrimary.xml "${${CLP}_SOURCE}" "${${CLP}_PRIMARY_SOURCE}" "VBRAINSDemonWarpPrimary" "BRAINSDemonWarpCommonLib")

### Add the Testing Subdirectory.
if(BUILD_TESTING)
  add_subdirectory(TestSuite)
endif(BUILD_TESTING)
if(NOT Slicer3_SOURCE_DIR)
  add_subdirectory(Example)
endif(NOT Slicer3_SOURCE_DIR)
