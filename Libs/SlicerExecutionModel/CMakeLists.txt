project(SlicerExecutionModel)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

if(DEFINED SlicerExecutionModel_INSTALL_BIN_DIR)
  set(GenerateCLP_INSTALL_BIN_DIR ${SlicerExecutionModel_INSTALL_BIN_DIR})
  set(ModuleDescriptionParser_INSTALL_BIN_DIR ${SlicerExecutionModel_INSTALL_BIN_DIR})
  set(tclap_INSTALL_BIN_DIR ${SlicerExecutionModel_INSTALL_BIN_DIR})
endif()

if(DEFINED SlicerExecutionModel_INSTALL_LIB_DIR)
  set(GenerateCLP_INSTALL_LIB_DIR ${SlicerExecutionModel_INSTALL_LIB_DIR})
  set(ModuleDescriptionParser_INSTALL_LIB_DIR ${SlicerExecutionModel_INSTALL_LIB_DIR})
  set(tclap_INSTALL_LIB_DIR ${SlicerExecutionModel_INSTALL_LIB_DIR})
endif()

if(DEFINED SlicerExecutionModel_INSTALL_NO_DEVELOPMENT)
  set(GenerateCLP_INSTALL_NO_DEVELOPMENT ${SlicerExecutionModel_INSTALL_NO_DEVELOPMENT})
  set(ModuleDescriptionParser_INSTALL_NO_DEVELOPMENT ${SlicerExecutionModel_INSTALL_NO_DEVELOPMENT})
  set(tclap_INSTALL_NO_DEVELOPMENT ${SlicerExecutionModel_INSTALL_NO_DEVELOPMENT})
endif()

if(DEFINED SlicerExecutionModel_EXPORT_FILE)
  set(GenerateCLP_EXPORT_FILE ${SlicerExecutionModel_EXPORT_FILE})
  set(ModuleDescriptionParser_EXPORT_FILE ${SlicerExecutionModel_EXPORT_FILE})
  #set(tclap_EXPORT_FILE ${SlicerExecutionModel_EXPORT_FILE})
endif()


# --------------------------------------------------------------------------
# SETIFEMPTY
# --------------------------------------------------------------------------
## A simple macro to set variables ONLY if it has not been set
## This is needed when stand-alone packages are combined into
## a larger package, and the desired behavior is that all the
## binary results end up in the combined directory.
## The signature has the same signature as the set, it just checks if the
## value is already set before executing.
if(NOT SETIFEMPTY)
macro(SETIFEMPTY)
  set(KEY ${ARGV0})
  set(VALUE ${ARGV1})
  if(NOT ${KEY})
    set(${ARGV})
  endif(NOT ${KEY})
endmacro()
endif(NOT SETIFEMPTY)
###

SETIFEMPTY(TCLAP_DIR ${CMAKE_CURRENT_BINARY_DIR}/tclap CACHE PATH "Location of TCLAPConfig.cmake" FORCE)
add_subdirectory(tclap)

SETIFEMPTY(ModuleDescriptionParser_DIR ${CMAKE_CURRENT_BINARY_DIR}/ModuleDescriptionParser CACHE PATH "Location of ModuleDescriptionParserConfig.cmake" FORCE)
add_subdirectory(ModuleDescriptionParser)

SETIFEMPTY(GenerateCLP_DIR ${CMAKE_CURRENT_BINARY_DIR}/GenerateCLP CACHE PATH "Location of GenerateCLPConfig.cmake" FORCE)
add_subdirectory(GenerateCLP)

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${GenerateCLP_SOURCE_DIR}
  ${GenerateCLP_BINARY_DIR}
  ${TCLAP_SOURCE_DIR}
  ${TCLAP_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE
  )
