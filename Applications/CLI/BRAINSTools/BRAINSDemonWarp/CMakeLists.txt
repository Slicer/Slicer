project(BRAINSDemonWarp)
set(LOCAL_PROJECT_NAME BRAINSDemonWarp)
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

enable_testing()
include(Dart)
include(CPack)
include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBuildMacros.cmake)

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


# No need to add with nothing to compile. add_subdirectory(BRAINSFit_Common)
include(${BRAINS_CMAKE_HELPER_DIR}/IJMacros.txt)

###
configure_file(${BRAINSDemonWarp_SOURCE_DIR}/CTestCustom.ctest ${BRAINSDemonWarp_BINARY_DIR}/CTestCustom.ctest COPYONLY)
#add_subdirectory(Example)

include(${BRAINS_CMAKE_HELPER_DIR}/CMakeBRAINS3BuildMacros.cmake)
###

set(CLP VBRAINSDemonWarp)
set(${CLP}_SOURCE  VBRAINSDemonWarp.cxx VBRAINSDemonWarpPrimary.cxx)
CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} VBRAINSDemonWarpPrimary.xml "${${CLP}_SOURCE}" "")

set(CLP BRAINSDemonWarp)
set(${CLP}_SOURCE BRAINSDemonWarp.cxx BRAINSDemonWarpPrimary.cxx)
CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSDemonWarpPrimary.xml "${${CLP}_SOURCE}" "")

if(BRAINS_BUILD) ## ONLY BUILD FOR BACKWARDS COMPATIBILITY IN BRAINS ENVIRONMENT
  set(CLP BRAINSDemonWarpCLP) ## NOTE: This is completely redundant, but is needed to support all the old scripts that use the original name
  set( ${CLP}_SOURCE BRAINSDemonWarpCLP.cxx BRAINSDemonWarpPrimary.cxx)
  CONFIGUREBRAINSORSLICERPROPERTIES(${CLP} BRAINSDemonWarpPrimary.xml "${${CLP}_SOURCE}" "")
## HACK -- Need to write other backwards compatibility wrappers for the old names of BRAINSDemonWarpCLP
endif(BRAINS_BUILD) ## ONLY BUILD FOR BACKWARDS COMPATIBILITY IN BRAINS ENVIRONMENT

### Add the Testing Subdirectory.
#add_subdirectory(TestingBRAINSDemonWarp)

if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS        
    VBRAINSDemonWarp
    BRAINSDemonWarp
    )
  slicer3_install_plugins(${TARGETS})
endif (Slicer3_SOURCE_DIR)
