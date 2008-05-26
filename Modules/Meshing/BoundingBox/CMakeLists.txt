project(BoundingBox)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_modules_install_prefix()
endif(NOT Slicer3_SOURCE_DIR)

slicer3_set_default_modules_output_path()

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MimxMeshing_SOURCE_DIR}/Common 
  ${MimxMeshing_BINARY_DIR}/Common 
  ${MimxMeshing_SOURCE_DIR}/Filter 
  ${MimxMeshing_BINARY_DIR}/Filter 
  ${MimxMeshing_SOURCE_DIR}/Widgets 
  ${MimxMeshing_BINARY_DIR}/Widgets 
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkBoundingBoxConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkBoundingBoxConfigure.h
)

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkBoundingBoxConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(BoundingBox_SRCS 
  vtkKWMimxApplication.cxx
  vtkKWMimxFEMeshMenuGroup.cxx
  vtkKWMimxGroupBase.cxx
  vtkKWMimxMainNotebook.cxx
  vtkKWMimxMainMenuGroup.cxx
  vtkKWMimxMenuWindow.cxx
  vtkKWMimxSurfaceMenuGroup.cxx
  vtkKWMimxViewWindow.cxx
  vtkKWMimxCreateBBFromBoundsGroup.cxx
  vtkKWMimxCreateBBMeshSeedGroup.cxx
  vtkKWMimxEditBBGroup.cxx
  vtkKWMimxCreateFEMeshFromBBGroup.cxx
  vtkKWMimxViewProperties.cxx
  vtkKWMimxSaveSTLSurfaceGroup.cxx
  vtkKWMimxSaveVTKSurfaceGroup.cxx
  vtkKWMimxSaveVTKBBGroup.cxx
  vtkKWMimxEditBBMeshSeedGroup.cxx
  vtkKWMimxSaveVTKFEMeshGroup.cxx
  vtkKWMimxDeleteObjectGroup.cxx
  vtkKWMimxEditFEMeshLaplacianSmoothGroup.cxx
  vtkMimxAbaqusFileWriter.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
kwwidgets_wrap_tcl(BoundingBox 
  BoundingBox_TCL_SRCS 
  "${BoundingBox_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name BoundingBox)
add_library(${lib_name} 
  ${BoundingBox_SRCS}
  ${BoundingBox_TCL_SRCS} 
)

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})
