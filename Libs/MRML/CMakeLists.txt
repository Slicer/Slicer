project(MRML)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMRMLConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMRMLConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkMRMLConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
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
  vtkMRMLBSplineTransformNode.cxx
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
# Wrapping

set(VTK_WRAP_HINTS ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping/Tcl/hints)
vtk_wrap_tcl3(MRML MRML_TCL_SRCS "${MRML_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name MRML)
add_library(${lib_name} 
  ${MRML_SRCS} 
  ${MRML_TCL_SRCS}
  )
target_link_libraries(${lib_name}
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

slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES ${tmp} ${lib_name})

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

# Install the testing data

file(GLOB_RECURSE MRMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.mrml")
file(GLOB_RECURSE TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.tcl")
file(GLOB_RECURSE XMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.xml")
file(GLOB_RECURSE GZFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.gz")
file(GLOB_RECURSE NHDRFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.nhdr")
file(GLOB_RECURSE NRRDFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.nrrd")
file(GLOB_RECURSE VTKFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.vtk")

foreach(file 
    ${MRMLFILES}
    ${TCLFILES}
    ${XMLFILES}
    ${GZFILES}
    ${NHDRFILES}
    ${NRRDFILES}
    ${VTKFILES}
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/share/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file) 

install(DIRECTORY
  Testing
  DESTINATION share/${PROJECT_NAME} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
