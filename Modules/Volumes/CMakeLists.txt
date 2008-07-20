project(Volumes)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

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
  ${CommandLineModule_SOURCE_DIR}
  ${CommandLineModule_BINARY_DIR}
  ${SlicerTractographyFiducialSeeding_SOURCE_DIR}
  ${SlicerTractographyFiducialSeeding_BINARY_DIR}
  ${SlicerTractographyDisplay_SOURCE_DIR}
  ${SlicerTractographyDisplay_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkVolumesConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkVolumesConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkVolumesConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(Volumes_SRCS
  vtkSlicerVolumesGUI.cxx
  vtkSlicerVolumesLogic.cxx
  #dwi/dti editor
  vtkSlicerDiffusionEditorWidget.cxx
  vtkSlicerMeasurementFrameWidget.cxx
  vtkSlicerGradientsWidget.cxx
  vtkSlicerDiffusionTestingWidget.cxx
  vtkSlicerDiffusionEditorLogic.cxx
  #display widgets
  vtkSlicerVolumeDisplayWidget.cxx
  vtkSlicerLabelMapVolumeDisplayWidget.cxx
  vtkSlicerScalarVolumeDisplayWidget.cxx
  vtkSlicerDiffusionWeightedVolumeDisplayWidget.cxx
  vtkSlicerDiffusionTensorVolumeDisplayWidget.cxx
  vtkSlicerDiffusionTensorVolumeGlyphDisplayWidget.cxx
  vtkKWWindowLevelThresholdEditor.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(Volumes 
  Volumes_TCL_SRCS 
  "${Volumes_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(Volumes_SRCS SlicerVolumes.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name Volumes)
add_library(${lib_name}
  ${Volumes_SRCS}
  ${Volumes_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  CommandLineModule
  SlicerTractographyDisplay
  SlicerTractographyFiducialSeeding
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  add_test(DiffusionEditorTest ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DiffusionEditorTest.tcl)
  add_test(DiffusionEditorGradients ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DiffusionEditor_GradientsTest.tcl)
  add_test(DiffusionEditorGlyphsTracts ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DiffusionEditorGlyphsTractsTest.tcl)
  add_test(DiffusionEditorTensor ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DiffusionEditorTensorTest.tcl)
endif(BUILD_TESTING)

# Install testing data

file(GLOB_RECURSE TXTFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.txt")
file(GLOB_RECURSE TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.tcl")

foreach(file 
    ${TXTFILES}
    ${TCLFILES}
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file) 

install(DIRECTORY
  Testing
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
