PROJECT( SlicerExecutionModel )
set(LOCAL_PROJECT_NAME SlicerExecutionModel)
cmake_minimum_required(VERSION 2.6)
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.6)
  cmake_policy(VERSION 2.8)
endif("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.6)

##NOTE:  Only build these items if they have not already been identified.
##       This allows for these items to be skipped if building against Slicer versions

if(NOT ITK_FOUND)
   find_package(ITK REQUIRED)
   include(${ITK_USE_FILE})
endif(NOT ITK_FOUND)

## A simple macro to set variables ONLY if it has not been set
## This is needed when stand-alone packages are combined into
## a larger package, and the desired behavior is that all the
## binary results end up in the combined directory.
## The signature has the same signature as the set, it just checks if the value is already set before executing.
if(NOT SETIFEMPTY)
macro(SETIFEMPTY)
  set(KEY ${ARGV0})
  set(VALUE ${ARGV1})
  if(NOT ${KEY})
    set(${ARGV})
  endif(NOT ${KEY})
endmacro(SETIFEMPTY KEY VALUE)
endif(NOT SETIFEMPTY)
###


SETIFEMPTY(TCLAP_DIR ${CMAKE_CURRENT_BINARY_DIR}/tclap CACHE PATH "Location of TCLAPConfig.cmake" FORCE)
ADD_SUBDIRECTORY(tclap)

SETIFEMPTY(ModuleDescriptionParser_DIR ${CMAKE_CURRENT_BINARY_DIR}/ModuleDescriptionParser CACHE PATH "Location of ModuleDescriptionParserConfig.cmake" FORCE)
ADD_SUBDIRECTORY(ModuleDescriptionParser)

SETIFEMPTY(GenerateCLP_DIR ${CMAKE_CURRENT_BINARY_DIR}/GenerateCLP CACHE PATH "Location of GenerateCLPConfig.cmake" FORCE)
ADD_SUBDIRECTORY(GenerateCLP)
