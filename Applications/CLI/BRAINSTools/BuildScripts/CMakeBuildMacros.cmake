#  This file contains common Macros that are useful for controlling
#  hierarchial build where common components may be included in more
#  than one subdirectory, but we only want a single copy of it built.


## A simple macro to set variables ONLY if it has not been set
## This is needed when stand-alone packages are combined into
## a larger package, and the desired behavior is that all the
## binary results end up in the combined directory.
if(NOT SETIFEMPTY)
macro(SETIFEMPTY)
  set(KEY ${ARGV0})
  set(VALUE ${ARGV1})
  if(NOT ${KEY})
    set(${ARGV})
  endif(NOT ${KEY})
endmacro(SETIFEMPTY KEY VALUE)
endif(NOT SETIFEMPTY)


##  Before looking for ITK or VTK, or many other packages, determine if
##  we are supposed to be building against Slicer3, and if so, then
##  be sure to get the Slicer3 version of the software first.
if(NOT CHECKIFSLICER3BUILD)
macro(CHECKIFSLICER3BUILD)
  if(BUILD_AGAINST_SLICER3)  ## This is only required to build against slicer as an external module.
    find_package(Slicer3 REQUIRED)
    if (Slicer3_FOUND)
      include(${Slicer3_USE_FILE})
    else (Slicer3_FOUND)
      message(FATAL_ERROR "Cannot build without a Slicer3 build tree or a Slicer3 installation when BUILD_AGAINST_SLICER3 was specified. Please set Slicer3_DIR when BUILD_AGAINST_SLICER3:BOOL=ON.")
    endif (Slicer3_FOUND)
    slicer3_set_default_install_prefix_for_external_projects()
  endif(BUILD_AGAINST_SLICER3)
endmacro(CHECKIFSLICER3BUILD)
endif(NOT CHECKIFSLICER3BUILD)


##  In many cases sub-projects depending on SlicerExectuion Model
##  that can be built stand alone are combined in larger packages.
##  This logic will include SlicerExectionModel only if it
##  has not already been included by a previous package.
if(NOT ADDFIRSTSLICEREXECUTIONMODEL)
macro(ADDFIRSTSLICEREXECUTIONMODEL LOCAL_PROJECT_NAMESPACE)
  if( BUILDSEMFROM${LOCAL_PROJECT_NAMESPACE} OR NOT GenerateCLP_DIR )
    set(BUILDSEMFROM${LOCAL_PROJECT_NAMESPACE} ON CACHE BOOL "FLAG FOR BUILD SlicerExectionModel building to prevent recursion.")
    add_subdirectory(SlicerExecutionModel)
  endif( BUILDSEMFROM${LOCAL_PROJECT_NAMESPACE} OR NOT GenerateCLP_DIR )

  find_package(GenerateCLP NO_MODULE REQUIRED)
  if(GenerateCLP_DIR)
    include(${GenerateCLP_USE_FILE})
  else(GenerateCLP_DIR)
    message(FATAL_ERROR "Can't build without GenerateCLP. Please set GenerateCLP_DIR")
  endif(GenerateCLP_DIR)

endmacro(ADDFIRSTSLICEREXECUTIONMODEL)
endif(NOT ADDFIRSTSLICEREXECUTIONMODEL)

##  In many cases stand-alone sub-projects include private versions
##  of DicomToNrrd
##  that can be built stand alone are combined in larger packages.
##  This logic will include DicomToNrrd only if it
##  has not already been included by a previous package.
if(NOT ADDFIRSTINSTANCE_DIRECTORY)
macro(ADDFIRSTINSTANCE_DIRECTORY PROJECT_NAMESPACE STANDALONENAME)
  if(BUILD${STANDALONENAME}FROM${PROJECT_NAMESPACE} OR NOT ${STANDALONENAME}_ALREADYINCLUDED)
    set(BUILD${STANDALONENAME}FROM${PROJECT_NAMESPACE} ON CACHE BOOL "FLAG FOR ${STANDALONENAME} building to prevent recursion.")
    set(${STANDALONENAME}_ALREADYINCLUDED ON CACHE BOOL "FLAG FOR ${STANDALONENAME} to indicate that it is already included.")
    add_subdirectory(${STANDALONENAME})
  endif( BUILD${STANDALONENAME}FROM${PROJECT_NAMESPACE} OR NOT ${STANDALONENAME}_ALREADYINCLUDED)
endmacro(ADDFIRSTINSTANCE_DIRECTORY)
endif(NOT ADDFIRSTINSTANCE_DIRECTORY)


