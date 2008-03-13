PROJECT(Volumes)

# --------------------------------------------------------------------------
# Sources
  SET(Volumes_SRCS
  vtkSlicerVolumesGUI.cxx
  #gradient editor
  vtkSlicerGradientEditorWidget.cxx
  vtkSlicerMeasurementFrameWidget.cxx
  vtkSlicerGradientsWidget.cxx
  vtkSlicerGradientEditorLogic.cxx
  #display widgets
  vtkSlicerVolumeDisplayWidget.cxx
  vtkSlicerLabelMapVolumeDisplayWidget.cxx
  vtkSlicerScalarVolumeDisplayWidget.cxx
  vtkSlicerDiffusionWeightedVolumeDisplayWidget.cxx
  vtkSlicerDiffusionTensorVolumeDisplayWidget.cxx
  vtkKWWindowLevelThresholdEditor.cxx
  )
#-----------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${TEEM_INCLUDE_DIR}
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${CommandLineModule_SOURCE_DIR}
  ${CommandLineModule_BINARY_DIR}
  ${SlicerTractographyFiducialSeeding_SOURCE_DIR}
  ${SlicerTractographyFiducialSeeding_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  )


# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(Volumes Volumes_TCL_SRCS "${Volumes_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(Volumes
  ${Volumes_SRCS}
  ${Volumes_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS Volumes
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(Volumes
  ${KWWidgets_LIBRARIES} 
  SlicerBaseLogic 
  SlicerBaseGUI
  MRML
  CommandLineModule
  SlicerTractographyFiducialSeeding
  ModuleDescriptionParser
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

CONFIGURE_FILE(
  ${Volumes_SOURCE_DIR}/vtkVolumesConfigure.h.in 
  ${Volumes_BINARY_DIR}/vtkVolumesConfigure.h
)
