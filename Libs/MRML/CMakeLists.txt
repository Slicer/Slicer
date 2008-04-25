project(MRML)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
configure_file(
  ${MRML_SOURCE_DIR}/vtkMRMLConfigure.h.in 
  ${MRML_BINARY_DIR}/vtkMRMLConfigure.h
  )

# --------------------------------------------------------------------------
# Sources

set(MRML_SRCS
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

if(TEEM_FOUND)
  set(MRML_SRCS
    ${MRML_SRCS}
    vtkMRMLNRRDStorageNode.cxx
    )
endif(TEEM_FOUND)

# Abstract/pure virtual classes

set_source_files_properties(
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

set(include_dirs
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
set(VTK_WRAP_HINTS ${MRML_SOURCE_DIR}/Wrapping/Tcl/hints)
vtk_wrap_tcl3(MRML MRML_TCL_SRCS "${MRML_SRCS}" "")

# Build the library

add_library(MRML 
  ${MRML_SRCS} ${MRML_TCL_SRCS}
  )

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} MRML)

if(BUILD_SHARED_LIBS)
  install(TARGETS MRML
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/MRML COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/MRML COMPONENT Development)
endif(BUILD_SHARED_LIBS)

target_link_libraries(MRML
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

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

file(GLOB __files1 "${MRML_SOURCE_DIR}/*.h")
install(FILES ${__files1} DESTINATION include/MRML COMPONENT Development)
install(FILES ${MRML_BINARY_DIR}/vtkMRMLConfigure.h DESTINATION include/MRML COMPONENT Development)
