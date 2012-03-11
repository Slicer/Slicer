
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED EMSegment_SOURCE_DIR AND NOT EXISTS ${EMSegment_SOURCE_DIR})
  message(FATAL_ERROR "EMSegment_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(EMSegment_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(EMSegment)
set(proj EMSegment)

if(NOT DEFINED EMSegment_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
    SVN_REPOSITORY "http://svn.slicer.org/Slicer3/trunk/Modules/EMSegment"
    SVN_REVISION -r "16973"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    UPDATE_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${EMSegment_DEPENDENCIES}
    )
  set(EMSegment_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using EMSegment_DIR, nevertheless since other project may depend on EMSegment,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${EMSegment_DEPENDENCIES}")
endif()

