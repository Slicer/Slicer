PROJECT(VolumeRenderingReplacements)
# --------------------------------------------------------------------------
# Sources
  SET(VolumeRenderingReplacements_SRCS
  vtkSlicerVolumePropertyWidget.cxx
  vtkSlicerVolumeTextureMapper3D.cxx
  vtkSlicerOpenGLVolumeTextureMapper3D.cxx
  vtkSlicerVolumeRenderingFactory.cxx
  #Ray Cast stuff
  vtkSlicerFixedPointRayCastImage.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeGOHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeShadeHelper.cxx
  vtkSlicerFixedPointVolumeRayCastHelper.cxx
  vtkSlicerFixedPointVolumeRayCastMapper.cxx
  vtkSlicerFixedPointVolumeRayCastMIPHelper.cxx
  vtkSlicerRayCastImageDisplayHelper.cxx
  vtkSlicerOpenGLRayCastImageDisplayHelper.cxx
  #LabelmapPart
  )
#-----------------------------------------------
#Include DIRs

INCLUDE_DIRECTORIES(
  ${VolumeRenderingReplacements_SOURCE_DIR}
  ${VolumeRenderingReplacements_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  )


# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(VolumeRenderingReplacements VolumeRenderingReplacements_TCL_SRCS "${VolumeRenderingReplacements_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(VolumeRenderingReplacements
  ${VolumeRenderingReplacements_SRCS}
  ${VolumeRenderingReplacements_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS VolumeRenderingReplacements
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(VolumeRenderingReplacements
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkITK
  vtkRendering
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  ${KWWidgets_LIBRARIES} )
  
  CONFIGURE_FILE(
  ${VolumeRenderingReplacements_SOURCE_DIR}/vtkVolumeRenderingReplacementsConfigure.h.in 
  ${VolumeRenderingReplacements_BINARY_DIR}/vtkVolumeRenderingReplacementsConfigure.h
)
