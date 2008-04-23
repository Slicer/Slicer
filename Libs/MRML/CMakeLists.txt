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
        vtkObserverManager.cxx
        vtkEventBroker.cxx
        vtkObservation.cxx
        vtkImageBimodalAnalysis.cxx
        vtkImageAccumulateDiscrete.cxx
        vtkMRMLScene.cxx
        vtkMRMLParser.cxx
        vtkMRMLNode.cxx
        vtkMRMLVolumeNode.cxx
        vtkMRMLScalarVolumeNode.cxx        
        vtkMRMLVectorVolumeNode.cxx        
        vtkMRMLModelNode.cxx
        vtkMRMLModelStorageNode.cxx
        vtkMRMLFreeSurferModelStorageNode.cxx    
        vtkMRMLFreeSurferModelOverlayStorageNode.cxx
        vtkMRMLModelDisplayNode.cxx
        vtkMRMLClipModelsNode.cxx
        vtkMRMLStorageNode.cxx
        vtkMRMLVolumeArchetypeStorageNode.cxx
        vtkMRMLVolumeHeaderlessStorageNode.cxx
        vtkMRMLVolumeDisplayNode.cxx
        vtkMRMLScalarVolumeDisplayNode.cxx
        vtkMRMLLabelMapVolumeDisplayNode.cxx
        vtkMRMLVolumeGlyphDisplayNode.cxx
        vtkMRMLTransformableNode.cxx
        vtkMRMLTransformNode.cxx
        vtkMRMLTransformStorageNode.cxx
        vtkMRMLLinearTransformNode.cxx
        vtkMRMLFiducial.cxx
        vtkMRMLFiducialListNode.cxx
        vtkMRMLSliceNode.cxx
        vtkMRMLSliceCompositeNode.cxx
        vtkMRMLSelectionNode.cxx
        vtkMRMLInteractionNode.cxx
        vtkMRMLColorNode.cxx
        vtkMRMLColorTableStorageNode.cxx
        vtkMRMLColorTableNode.cxx
        vtkMRMLProceduralColorNode.cxx
        vtkMRMLFreeSurferProceduralColorNode.cxx
        vtkMRMLCameraNode.cxx
        vtkMRMLViewNode.cxx
        vtkMRMLTensorVolumeNode.cxx
        vtkMRMLTimeSeriesVolumeNode.cxx
        vtkMRMLDiffusionTensorDisplayPropertiesNode.cxx
        vtkMRMLDiffusionWeightedVolumeDisplayNode.cxx
        vtkMRMLDiffusionWeightedVolumeNode.cxx
        vtkMRMLDiffusionImageVolumeNode.cxx
        vtkMRMLDiffusionTensorVolumeNode.cxx
        vtkMRMLDiffusionTensorVolumeDisplayNode.cxx
        vtkMRMLDiffusionTensorVolumeSliceDisplayNode.cxx
        vtkMRMLVectorVolumeDisplayNode.cxx
        vtkMRMLFiberBundleNode.cxx
        vtkMRMLFiberBundleDisplayNode.cxx
        vtkMRMLFiberBundleGlyphDisplayNode.cxx
        vtkMRMLFiberBundleTubeDisplayNode.cxx
        vtkMRMLFiberBundleLineDisplayNode.cxx
        vtkMRMLFiberBundleStorageNode.cxx
        vtkMRMLHierarchyNode.cxx
        vtkMRMLModelHierarchyNode.cxx
        vtkMRMLSceneSnapshotNode.cxx
        vtkMRMLSnapshotClipNode.cxx
        vtkMRMLROINode.cxx
        vtkMRMLROIListNode.cxx
        vtkMRMLUnstructuredGridNode.cxx
        vtkMRMLUnstructuredGridDisplayNode.cxx
        vtkMRMLUnstructuredGridStorageNode.cxx
        vtkMRMLDisplayNode.cxx
        vtkMRMLDisplayableNode.cxx
        vtkMRMLStorableNode.cxx
        # Classes for remote data handling:
        vtkDataIOManager.cxx
        vtkDataTransfer.cxx
        vtkURIHandler.cxx
        vtkCacheManager.cxx
        vtkPermissionPrompter.cxx
        vtkMRMLNonlinearTransformNode.cxx
        vtkMRMLGridTransformNode.cxx
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
  vtkMRMLDisplayNode.cxx
  vtkMRMLDisplayableNode.cxx
  vtkMRMLVolumeDisplayNode.cxx
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
  INSTALL(TARGETS MRML
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/MRML COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/MRML COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(MRML
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkGraphicsTCL
  vtkRenderingTCL
  vtkHybridTCL
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
  vtkTeem
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

FILE(GLOB __files1 "${MRML_SOURCE_DIR}/*.h")
INSTALL(FILES ${__files1} DESTINATION include/MRML COMPONENT Development)
INSTALL(FILES ${MRML_BINARY_DIR}/vtkMRMLConfigure.h DESTINATION include/MRML COMPONENT Development)
