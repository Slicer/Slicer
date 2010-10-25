project(QueryAtlas)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

# This module use's FreeType, which is not quite exported correctly by an
# installed VTK. It's not obvious either if this should be exposed 
# directly by VTK, but this is an easy fix.

if(VTK_INSTALL_PREFIX)
  include_directories(
    ${VTK_INSTALL_PREFIX}/${VTK_INSTALL_INCLUDE_DIR}/vtkfreetype/include
    )
endif(VTK_INSTALL_PREFIX)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkQueryAtlasConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkQueryAtlasConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkQueryAtlasConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h")
install(FILES 
  ${headers} 
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  ) 

# --------------------------------------------------------------------------
# Sources

set(QueryAtlas_SRCS
  vtkQueryAtlasIcons.cxx        
  vtkQueryAtlasLogic.cxx
  vtkQueryAtlasGUI.cxx
  vtkMRMLQueryAtlasNode.cxx
  vtkQueryAtlasCollaboratorIcons.cxx
  vtkQueryAtlasUseSearchTermWidget.cxx  
  vtkQueryAtlasSearchTermWidget.cxx  
  # cards and related classes
  vtkBIRNCard.cxx
  vtkBIRNCardManager.cxx
  vtkCard.cxx
  vtkCardManager.cxx
  vtkEdgeGradient.cxx
  vtkFontParameters.cxx
  vtkFreetypeRasterizer.cxx
  vtkRasterizerCharacter.cxx
  vtkRectangle.cxx
  #vtkSortCommand.cxx
  vtkSorter.cxx
  vtkTextureFont.cxx
  vtkTextureFontManager.cxx
  vtkTextureText.cxx
  vtkTextureTextPolys.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(QueryAtlas QueryAtlas_TCL_SRCS "${QueryAtlas_SRCS}" "")

# --------------------------------------------------------------------------
# Add Loadable Module support

generatelm(QueryAtlas_SRCS QueryAtlas.xml)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name QueryAtlas)
add_library(${lib_name}
  ${QueryAtlas_SRCS}
  ${QueryAtlas_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

# Should those paths change, make sure to update 
#   - Tcl/QueryAtlas.tcl
#   - Tcl/QueryAtlasControlledVocabulary.tcl
#   - vtkQueryAtlasGUI.cxx

# Tcl files

# Question: I'm not quite sure if the Tcl scripts here are supposed to be a module loaded at run-time; since there is a pkgindex.tcl I assume it was a Tcl "module", one loaded by ScriptedModule, but said ScriptedModule does choke at run-time trying to call QueryAtlasBuildGUI or QueryAtlasAddGUIObservers (both methods created by appending "BuildGUI" or "AddGUIObservers" to the module name, none of them to be found in the scripts here). Until we find out, let's copy those support files to share/ instead of lib/.  vtkQueryAtlasGUI.cxx seems to add the path to these scripts manually by updating auto_path, so maybe they are just support scripts, but part of a standalone module.

file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tcl/*.tcl")

foreach(file ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Tcl
  )

# Resources (CSV, txt, xls, .h)

file(GLOB CSVFILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.csv")
file(GLOB TXTFILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.txt")
file(GLOB XLSFILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.xls")

foreach(file 
    ${CSVFILES} 
    ${TXTFILES}
    ${XLSFILES}
)
  get_filename_component(filename "${file}" NAME)
  configure_file(
    ${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Resources/${filename}
    COPYONLY)
endforeach(file)

install(FILES 
  ${CSVFILES} 
  ${TXTFILES} 
  ${XLSFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  ) 

# ImageData

file(GLOB IMAGEFILES "${CMAKE_CURRENT_SOURCE_DIR}/ImageData/*.png")

foreach(file ${IMAGEFILES})
  get_filename_component(filename "${file}" NAME)
  configure_file(
    ${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/ImageData/${filename}
    COPYONLY)
endforeach(file)

install(FILES 
  ${IMAGEFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/ImageData COMPONENT Development
  ) 

# Ontoviz

set(ONTOVIZFILES
  OntologyViz/bin/birnlexvis-support.jar
  OntologyViz/bin/birnlexvis.jar
  OntologyViz/bin/birnlexvis.tcl
  OntologyViz/bin/birnlexvisUnix.sh
  OntologyViz/bin/birnlexvisWin.sh
  OntologyViz/build.xml
  OntologyViz/data/birnlex-body-v1.json
  OntologyViz/data/neuronames-v1.json
  OntologyViz/extjars/json.jar
  OntologyViz/extjars/jython.jar
  OntologyViz/extjars/prefuse.jar    
  )

foreach(file ${ONTOVIZFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
  get_filename_component(path "${file}" PATH)
  install(FILES 
    ${file} 
    DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${path} COMPONENT Development
    ) 
endforeach(file)
