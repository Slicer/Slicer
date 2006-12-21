PROJECT(MRML)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
CONFIGURE_FILE(
  ${MRML_SOURCE_DIR}/vtkMRMLConfigure.h.in 
  ${MRML_BINARY_DIR}/vtkMRMLConfigure.h
)


# --------------------------------------------------------------------------
# Sources

SET(MRML_SRCS
        vtkMRMLScene.cxx
        vtkMRMLParser.cxx
        vtkMRMLNode.cxx
        vtkMRMLVolumeNode.cxx
        vtkMRMLScalarVolumeNode.cxx        
        vtkMRMLVectorVolumeNode.cxx        
        vtkMRMLModelNode.cxx
        vtkMRMLModelStorageNode.cxx
        vtkMRMLModelDisplayNode.cxx
        vtkMRMLClipModelsNode.cxx
        vtkMRMLStorageNode.cxx
        vtkMRMLVolumeArchetypeStorageNode.cxx
        vtkMRMLVolumeHeaderlessStorageNode.cxx
        vtkMRMLVolumeDisplayNode.cxx
        vtkMRMLTransformableNode.cxx
        vtkMRMLTransformNode.cxx
        vtkMRMLLinearTransformNode.cxx
        vtkMRMLFiducial.cxx
        vtkMRMLFiducialListNode.cxx
        vtkMRMLSliceNode.cxx
        vtkMRMLSliceCompositeNode.cxx
        vtkMRMLSelectionNode.cxx
        vtkObserverManager.cxx
        vtkMRMLColorNode.cxx
        vtkMRMLCameraNode.cxx
        vtkMRMLViewNode.cxx
        vtkMRMLTensorVolumeNode.cxx
        vtkMRMLDiffusionWeightedVolumeDisplayNode.cxx
        vtkMRMLDiffusionWeightedVolumeNode.cxx
        vtkMRMLDiffusionTensorVolumeNode.cxx
        vtkMRMLDiffusionTensorVolumeDisplayNode.cxx
        vtkMRMLVectorVolumeDisplayNode.cxx
        # Note: when you add new node types, add
        # register code to the vtkMRMLScene constructor
)

IF( TEEM_FOUND )
  SET(MRML_SRCS
    ${MRML_SRCS}
    vtkMRMLNRRDStorageNode.cxx
  )
ENDIF( TEEM_FOUND )

# Abstract/pure virtual classes

SET_SOURCE_FILES_PROPERTIES(
  vtkMRMLNode.cxx 
  vtkMRMLVolumeNode.cxx
  vtkMRMLTransformNode.cxx
  ABSTRACT
  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${MRML_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${TEEM_INCLUDE_DIRS}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )

# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
SET(VTK_WRAP_HINTS ${MRML_SOURCE_DIR}/Wrapping/Tcl/hints)
VTK_WRAP_TCL3(MRML MRML_TCL_SRCS "${MRML_SRCS}" "")


# Build the library

ADD_LIBRARY(MRML 
  ${MRML_SRCS} ${MRML_TCL_SRCS}
  )
IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} MRML)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(MRML
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkGraphicsTCL
  vtkRenderingTCL
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  FreeSurfer
  )

IF( TEEM_FOUND )
  TARGET_LINK_LIBRARIES(MRML
    vtkTeem
  )
ENDIF( TEEM_FOUND )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

