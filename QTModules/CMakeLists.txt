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
  Colors
  Data
  Transforms
  Annotations
  #Measurements
  Models
  #FiberBundles
  SceneViews
  SlicerWelcome
  Tractography
  TractographyDisplay
  Volumes
  TractographyFiducialSeeding # tractography depends of Volumes
  CropVolume
  VolumeRendering # Volume Rendering depends of Volumes
  )
if(BUILD_TESTING)
  list(APPEND qtmodules ModuleTemplate)
endif()

foreach(module ${qtmodules})
  add_subdirectory(${module})
endforeach(module)

# NOTE: EMSegment source code is checkout from Slicer3 repository using
#       "External_EMSegment.cmake".
add_subdirectory(
  ${EMSegment_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/EMSegment
  )
