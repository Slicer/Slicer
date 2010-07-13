CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


# Calling PROJECT() defines projectname_SOURCE_DIR and projectname_BINARY_DIR
PROJECT(QTModules)

IF(NOT Slicer3_SOURCE_DIR)
  FIND_PACKAGE(Slicer3 REQUIRED)
  INCLUDE(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
ENDIF(NOT Slicer3_SOURCE_DIR)

INCLUDE(${Slicer3_CMAKE_DIR}/Slicer3QTModuleMacros.cmake)

SET(qtmodules
  Annotation
  EMSegment
  Measurements
  SlicerWelcome
  TractographyDisplay
  TractographyFiducialSeeding
  Volumes
  )

FOREACH(module ${qtmodules})
  ADD_SUBDIRECTORY(${module})
ENDFOREACH(module)
