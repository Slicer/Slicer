# ============================================================================
#
#  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
#  Reserved.
#
#  See Doc/copyright/copyright.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Program:   3D Slicer
#  Module:    $RCSfile: CMakeLists.txt,v $
#  Date:      $Date: 2006/01/06 17:56:51 $
#  Version:   $Revision: 1.6 $
#  Author:    $Sylvain Jaume (MIT)$
#
# ============================================================================

PROJECT(AtlasCreator)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkAtlasCreatorConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkAtlasCreatorConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES
  ${headers}
  "${CMAKE_CURRENT_BINARY_DIR}/vtkAtlasCreatorConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME}
  COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(AtlasCreator_SRCS
  vtkAtlasCreatorLogic.cxx
  vtkAtlasCreatorGUI.cxx
  vtkMRMLAtlasCreatorNode.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(AtlasCreator
  AtlasCreator_TCL_SRCS
  "${AtlasCreator_SRCS}" ""
  )

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(AtlasCreator_SRCS
  AtlasCreator.xml
  )

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name AtlasCreator)

add_library(${lib_name}
  ${AtlasCreator_SRCS}
  ${AtlasCreator_TCL_SRCS}
  )

slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  )

slicer3_install_modules(${lib_name})

