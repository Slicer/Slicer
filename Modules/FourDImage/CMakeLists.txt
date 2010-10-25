PROJECT(FourDImage)

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
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  )

#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkFourDImageConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkFourDImageConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkFourDImageConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h") 
install(FILES 
  ${files}
  DESTINATION ${Slicer_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(FourDImage_SRCS 
  vtkFourDImageLogic.cxx
  vtkFourDImageGUI.cxx
  vtkMRMLTimeSeriesBundleNode.cxx
  vtkFourDImageIcons.cxx
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(FourDImage FourDImage_TCL_SRCS "${FourDImage_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(FourDImage_SRCS FourDImage.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name FourDImage)
add_library(${lib_name}
  ${FourDImage_SRCS} 
  ${FourDImage_TCL_SRCS}
  )

slicer3_set_modules_output_path(${lib_name})

find_library( VolumesLib Volumes ${Slicer_DIR}/${Slicer_INSTALL_MODULES_BIN_DIR} )
if ( ${VolumesLib} MATCHES "NOTFOUND" )
   set ( VolumesLib Volumes )
endif ( ${VolumesLib} MATCHES "NOTFOUND" )

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  ${CLP_MODULE}
  vtkSlicerVolumesModuleLogic
  ${VolumesLib}
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

slicer3_install_modules(${lib_name})
