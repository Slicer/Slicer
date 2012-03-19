
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED MultiVolumeExplorer_SOURCE_DIR AND NOT EXISTS ${MultiVolumeExplorer_SOURCE_DIR})
  message(FATAL_ERROR "MultiVolumeExplorer_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(MultiVolumeExplorer_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(MultiVolumeExplorer)
set(proj MultiVolumeExplorer)

if(NOT DEFINED MultiVolumeExplorer_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/jcfr/MultiVolumeExplorer.git"
    GIT_TAG "55230d0f14e59742e4d49ca950effc00154dad60"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    UPDATE_COMMAND ""
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${MultiVolumeExplorer_DEPENDENCIES}
    )
  set(MultiVolumeExplorer_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using MultiVolumeExplorer_DIR, nevertheless since other project may depend on EMSegment,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${MultiVolumeExplorer_DEPENDENCIES}")
endif()

