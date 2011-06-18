project(AtlasCreator)

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

# --------------------------------------------------------------------------
# Add subdirectories

ADD_SUBDIRECTORY(Logic)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${vtkSlicerAtlasCreatorModuleLogic_SOURCE_DIR}
  ${vtkSlicerAtlasCreatorModuleLogic_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkAtlasCreatorConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkAtlasCreatorConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkAtlasCreatorConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(AtlasCreator_SRCS
  vtkAtlasCreatorGUI.cxx
  ${SRCS}
  )

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#vtkModule.cxx
#PROPERTIES
#ABSTRACT "TRUE"
#)

# Helper classes

# SET_SOURCE_FILES_PROPERTIES(
#   WRAP_EXCLUDE
#   )


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(AtlasCreator 
  AtlasCreator_TCL_SRCS 
  "${AtlasCreator_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(AtlasCreator_SRCS 
  AtlasCreator.xml)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name AtlasCreator)
add_library(${lib_name}
  ${AtlasCreator_SRCS}
  ${AtlasCreator_TCL_SRCS}
  )
#slicer3_set_modules_output_path(${lib_name})
SET_TARGET_PROPERTIES(${lib_name} PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_BIN_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}"
  )

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  vtkSlicerAtlasCreatorModuleLogic
  )

#slicer3_install_modules(${lib_name})
INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing

IF(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
ENDIF()
