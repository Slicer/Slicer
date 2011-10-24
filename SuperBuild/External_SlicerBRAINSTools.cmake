
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED SlicerBRAINSTools_SOURCE_DIR AND NOT EXISTS ${SlicerBRAINSTools_SOURCE_DIR})
  message(FATAL_ERROR "SlicerBRAINSTools_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(SlicerBRAINSTools_DEPENDENCIES ${ITK_EXTERNAL_NAME} SlicerExecutionModel VTK )

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(SlicerBRAINSTools)
set(proj SlicerBRAINSTools)

# Set CMake OSX variable to pass down the external project
#set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
#if(APPLE)
#  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
#    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
#    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
#    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
#endif()

if(NOT DEFINED SlicerBRAINSTools_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/jcfr/BRAINSStandAlone.git"
    GIT_TAG "951d3aa58a93dd684c6b5322a58fb17745f62927"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${SlicerBRAINSTools_DEPENDENCIES}
    )
  set(SlicerBRAINSTools_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using SlicerBRAINSTools_DIR, nevertheless since other project may depend on SlicerBRAINSTools,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${SlicerBRAINSTools_DEPENDENCIES}")
endif()
