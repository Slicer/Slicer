PROJECT(FourDImage)

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
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h") 
install(FILES 
  ${files}
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
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

find_library( VolumesLib Volumes ${Slicer3_DIR}/${Slicer3_INSTALL_MODULES_BIN_DIR} )
if ( ${VolumesLib} MATCHES "NOTFOUND" )
   set ( VolumesLib Volumes )
endif ( ${VolumesLib} MATCHES "NOTFOUND" )

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  ${CLP_MODULE}
  vtkSlicerVolumesModuleLogic
  ${VolumesLib}
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

slicer3_install_modules(${lib_name})
