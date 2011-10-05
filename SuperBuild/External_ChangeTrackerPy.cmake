
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED ChangeTrackerPy_SOURCE_DIR AND NOT EXISTS ${ChangeTrackerPy_SOURCE_DIR})
  message(FATAL_ERROR "ChangeTrackerPy_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(ChangeTrackerPy_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(ChangeTrackerPy)
set(proj ChangeTrackerPy)

if(NOT DEFINED ChangeTrackerPy_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/fedorov/ChangeTrackerPy.git"
    GIT_TAG "1b6275f8e7f41b29b86e9758dd0346fae3737a6e"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${ChangeTrackerPy_DEPENDENCIES}
    )
  set(ChangeTrackerPy_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using ChangeTrackerPy_DIR, nevertheless since other project may depend on EMSegment,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${ChangeTrackerPy_DEPENDENCIES}")
endif()

