PROJECT(VolumeRenderingModule)
OPTION(VOLUMERENDERINGMODULE "Would you like to build Volume Rendering" OFF)
OPTION(VR_LABELMAP "Would you like to use Labelmaps" OFF)
if(VR_LABELMAP AND NOT VOLUMERENDERINGMODULE)
MESSAGE(FATAL_ERROR "VR_LABELMAP can only be build if VOLUMERENDERINGMODULE is ON")
ENDIF(VR_LABELMAP AND NOT VOLUMERENDERINGMODULE)
if(VOLUMERENDERINGMODULE)
# --------------------------------------------------------------------------
# Sources
if(VR_LABELMAP)
  SET(VolumeRenderingModule_SRCS
  vtkVolumeRenderingModuleGUI.cxx
  vtkVolumeRenderingModuleLogic.cxx
  vtkMRMLVolumeRenderingNode.cxx
  vtkSlicerVolumePropertyWidget.cxx
  vtkSlicerNodeSelectorVolumeRenderingWidget.cxx
  vtkSlicerVRHelper.cxx
  vtkSlicerVRGrayscaleHelper.cxx
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
  vtkSlicerVRLabelmapHelper.cxx
  vtkLabelMapColorTransferFunction.cxx
  vtkLabelMapPiecewiseFunction.cxx
  vtkSlicerLabelMapWidget.cxx
  vtkEffectedArea.cxx)
ELSE(VR_LABELMAP)
ADD_DEFINITIONS(-DVR_LABELMAP_DEBUG)
  SET(VolumeRenderingModule_SRCS
  vtkVolumeRenderingModuleGUI.cxx
  vtkVolumeRenderingModuleLogic.cxx
  vtkMRMLVolumeRenderingNode.cxx
  vtkSlicerVolumePropertyWidget.cxx
  vtkSlicerNodeSelectorVolumeRenderingWidget.cxx
  vtkSlicerVRHelper.cxx
  vtkSlicerVRGrayscaleHelper.cxx
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
  vtkSlicerOpenGLRayCastImageDisplayHelper.cxx)
ENDIF(VR_LABELMAP)
# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
  #vtkModule.cxx
  #PROPERTIES
  #ABSTRACT "TRUE"
  #)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
  #vtkModule.cxx
  #PROPERTIES
  #WRAP_EXCLUDE "TRUE"
  #)

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${VolumeRenderingModule_SOURCE_DIR}
  ${VolumeRenderingModule_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )


# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(VolumeRenderingModule VolumeRenderingModule_TCL_SRCS "${VolumeRenderingModule_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(VolumeRenderingModule
  ${VolumeRenderingModule_SRCS}
  ${VolumeRenderingModule_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS VolumeRenderingModule
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(VolumeRenderingModule
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkITK
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



# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${VolumeRenderingModule_SOURCE_DIR}/vtkVolumeRenderingModuleConfigure.h.in 
  ${VolumeRenderingModule_BINARY_DIR}/vtkVolumeRenderingModuleConfigure.h
)
CONFIGURE_FILE(
  ${VolumeRenderingModule_SOURCE_DIR}/presets.xml
  ${VolumeRenderingModule_BINARY_DIR}/presets.xml
  COPYONLY
)
ENDIF(VOLUMERENDERINGMODULE)
