#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()


# Calling project() defines projectname_SOURCE_DIR and projectname_BINARY_DIR
project(QTModules)

include(${Slicer_CMAKE_DIR}/SlicerMacroBuildQtModule.cmake)

set(qtmodules
  Annotations
  #Measurements
  Models
  #FiberBundles
  SceneViews
  SlicerWelcome
  TractographyDisplay
  TractographyFiducialSeeding
  Volumes
  CropVolume
  VolumeRendering
  )
if(BUILD_TESTING)
  list(APPEND qtmodules ModuleTemplate)
endif()

foreach(module ${qtmodules})
  add_subdirectory(${module})
endforeach(module)
