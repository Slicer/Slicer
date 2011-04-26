#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()


# Calling PROJECT() defines projectname_SOURCE_DIR and projectname_BINARY_DIR
PROJECT(QTModules)

INCLUDE(${Slicer_CMAKE_DIR}/SlicerMacroBuildQtModule.cmake)

SET(qtmodules
  Annotation
  #Measurements
  Models
  #FiberBundles
  SceneViews
  SlicerWelcome
  TractographyDisplay
  TractographyFiducialSeeding
  Volumes
  #CropVolume
  VolumeRendering
  )

FOREACH(module ${qtmodules})
  ADD_SUBDIRECTORY(${module})
ENDFOREACH(module)
