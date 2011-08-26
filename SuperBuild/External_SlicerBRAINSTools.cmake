
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED SlicerBRAINSTools_DIR AND NOT EXISTS ${SlicerBRAINSTools_DIR})
  message(FATAL_ERROR "SlicerBRAINSTools_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(SlicerBRAINSTools_DEPENDENCIES ${ITK_EXTERNAL_NAME} SlicerExecutionModel VTK )

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(SlicerBRAINSTools)
set(proj SlicerBRAINSTools)

# Set CMake OSX variable to pass down the external project
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

if(NOT DEFINED SlicerBRAINSTools_DIR)

#-----------------------------------------------------------------------------
# DUPLICATED THERE FOR COMPLETNESS Plugins and Modules relative directories
#------------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Slicer install directories
#-----------------------------------------------------------------------------
set(Slicer_INSTALL_ROOT "./")
if(APPLE)
  # Set to create Bundle
  set(Slicer_BUNDLE_LOCATION "Slicer.app/Contents")
  set(Slicer_INSTALL_ROOT "${Slicer_BUNDLE_LOCATION}/")
endif()

set(Slicer_INSTALL_PLUGINS_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_BIN_DIR}")
set(Slicer_INSTALL_PLUGINS_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_PLUGINS_LIB_DIR}")

set(Slicer_HOME "${Slicer_BINARY_DIR}")
  #message(STATUS "${__indent}Adding project ${proj}")
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/BRAINSia/BRAINSStandAlone.git"
    GIT_TAG "master"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags} # Unused
    #################### Propogate the Slicer Environment
    ## -- This could be some other variable to indicate a slicer build
      -DINTEGRATE_WITH_SLICER:BOOL=ON
      -DBRAINSTools_SUPERBUILD:BOOL=OFF
      -DSlicer_PLUGINS_BIN_DIR:PATH=${CMAKE_BINARY_DIR}/Slicer-build/${Slicer_CLIMODULES_BIN_DIR}
      -DSlicer_PLUGINS_LIB_DIR:PATH=${CMAKE_BINARY_DIR}/Slicer-build/${Slicer_CLIMODULES_LIB_DIR}
      -DSlicer_INSTALL_PLUGINS_BIN_DIR:PATH=${CMAKE_BINARY_DIR}/Slicer-build/${Slicer_INSTALL_PLUGINS_BIN_DIR}
      -DSlicer_INSTALL_PLUGINS_LIB_DIR:PATH=${CMAKE_BINARY_DIR}/Slicer-build/${Slicer_INSTALL_PLUGINS_LIB_DIR}
    #################### Propogate the Slicer Environment
    # ITK
    -DITK_DIR:PATH=${ITK_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}
    # SlicerExecutionModel
    -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
      ## Which SlicerBRAINSTools packages to use
      -DUSE_BRAINSFit:BOOL=ON
      -DUSE_BRAINSResample:BOOL=ON
      -DUSE_BRAINSROIAuto:BOOL=ON
      -DUSE_BRAINSDemonWarp:BOOL=ON
      -DUSE_BRAINSMultiModeSegment:BOOL=ON
      -DUSE_BRAINSMush:BOOL=ON
      -DUSE_BRAINSConstellationDetector:BOOL=ON
      -DUSE_GTRACT:BOOL=ON
      -DUSE_BRAINSABC:BOOL=ON
      -DUSE_BRAINSInitializedControlPoints:BOOL=ON
      -DBUILD_TESTING:BOOL=OFF
    INSTALL_COMMAND ""
    DEPENDS
      ${SlicerBRAINSTools_DEPENDENCIES}
    )
  set(SlicerBRAINSTools_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
else()
  # The project is provided using SlicerBRAINSTools_DIR, nevertheless since other project may depend on SlicerBRAINSTools,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${SlicerBRAINSTools_DEPENDENCIES}")
endif()

