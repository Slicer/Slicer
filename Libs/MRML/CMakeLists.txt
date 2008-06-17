project(MRML)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

option(MRML_USE_TEEM "Build MRML with TEEM support." ON)
mark_as_advanced(MRML_USE_TEEM)
if(MRML_USE_TEEM)
  find_package(TEEM REQUIRED)
  include(${TEEM_USE_FILE})
  option(MRML_USE_vtkTeem "Build MRML with vtkTeem support." ON)
  mark_as_advanced(MRML_USE_vtkTeem)
endif(MRML_USE_TEEM)

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(packages
  vtkITK 
  FreeSurfer
  )

if(MRML_USE_TEEM)
  set(packages ${packages} 
    vtkTeem
    )
endif(MRML_USE_TEEM)
  
foreach(package 
    ${packages}
    )
  # This is mostly to test building this lib as standalone outside Slicer3
  if(NOT DEFINED ${package}_SOURCE_DIR)
    set(${package}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${package}")
  endif(NOT DEFINED ${package}_SOURCE_DIR)
  if(NOT DEFINED ${package}_BINARY_DIR)
    string(REPLACE ${PROJECT_NAME} ${package} 
      ${package}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    link_directories(${${package}_BINARY_DIR})
  endif(NOT DEFINED ${package}_BINARY_DIR)

  set(include_dirs ${include_dirs}
    ${${package}_SOURCE_DIR}
    ${${package}_BINARY_DIR}
    )
endforeach(package)

include_directories(${include_dirs})

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
  vtkEventBroker.cxx
  vtkImageAccumulateDiscrete.cxx
  vtkImageBimodalAnalysis.cxx
  vtkMRMLCameraNode.cxx
  vtkMRMLClipModelsNode.cxx
  vtkMRMLColorNode.cxx
  vtkMRMLColorTableNode.cxx
  vtkMRMLColorTableStorageNode.cxx
  vtkMRMLDiffusionTensorDisplayPropertiesNode.cxx
  vtkMRMLDiffusionWeightedVolumeDisplayNode.cxx
  vtkMRMLDiffusionWeightedVolumeNode.cxx
  vtkMRMLDisplayNode.cxx
  vtkMRMLDisplayableNode.cxx
  vtkMRMLFiberBundleDisplayNode.cxx
  vtkMRMLFiducial.cxx
  vtkMRMLFiducialListNode.cxx
  vtkMRMLFreeSurferModelOverlayStorageNode.cxx
  vtkMRMLFreeSurferModelStorageNode.cxx    
  vtkMRMLFreeSurferProceduralColorNode.cxx
  vtkMRMLHierarchyNode.cxx
  vtkMRMLInteractionNode.cxx
  vtkMRMLLabelMapVolumeDisplayNode.cxx
  vtkMRMLLinearTransformNode.cxx
  vtkMRMLModelDisplayNode.cxx
  vtkMRMLModelHierarchyNode.cxx
  vtkMRMLModelNode.cxx
  vtkMRMLModelStorageNode.cxx
  vtkMRMLNode.cxx
  vtkMRMLParser.cxx
  vtkMRMLProceduralColorNode.cxx
  vtkMRMLROIListNode.cxx
  vtkMRMLROINode.cxx
  vtkMRMLScalarVolumeDisplayNode.cxx
  vtkMRMLScalarVolumeNode.cxx        
  vtkMRMLScene.cxx
  vtkMRMLSceneSnapshotNode.cxx
  vtkMRMLSelectionNode.cxx
  vtkMRMLSliceCompositeNode.cxx
  vtkMRMLSliceNode.cxx
  vtkMRMLSnapshotClipNode.cxx
  vtkMRMLStorableNode.cxx
  vtkMRMLStorageNode.cxx
  vtkMRMLTimeSeriesVolumeNode.cxx
  vtkMRMLTransformNode.cxx
  vtkMRMLTransformStorageNode.cxx
  vtkMRMLTransformableNode.cxx
  vtkMRMLUnstructuredGridDisplayNode.cxx
  vtkMRMLUnstructuredGridNode.cxx
  vtkMRMLUnstructuredGridStorageNode.cxx
  vtkMRMLVectorVolumeDisplayNode.cxx
  vtkMRMLViewNode.cxx
  vtkMRMLVolumeArchetypeStorageNode.cxx
  vtkMRMLVolumeDisplayNode.cxx
  vtkMRMLVolumeGlyphDisplayNode.cxx
  vtkMRMLVolumeHeaderlessStorageNode.cxx
  vtkMRMLVolumeNode.cxx
  vtkObservation.cxx
  vtkObserverManager.cxx
  vtkMRMLLayoutNode.cxx
  # Classes for remote data handling:
  vtkCacheManager.cxx
  vtkDataIOManager.cxx
  vtkDataTransfer.cxx
  vtkMRMLBSplineTransformNode.cxx
  vtkMRMLGridTransformNode.cxx
  vtkMRMLNonlinearTransformNode.cxx
  vtkPermissionPrompter.cxx
  vtkURIHandler.cxx
  # Note: when you add new node types, add
  # register code to the vtkMRMLScene constructor
  )

if(MRML_USE_TEEM)
  if(MRML_USE_vtkTeem)
    set(MRML_SRCS
      ${MRML_SRCS}
      vtkMRMLDiffusionImageVolumeNode.cxx
      vtkMRMLDiffusionTensorVolumeDisplayNode.cxx
      vtkMRMLDiffusionTensorVolumeNode.cxx
      vtkMRMLDiffusionTensorVolumeSliceDisplayNode.cxx
      vtkMRMLFiberBundleGlyphDisplayNode.cxx
      vtkMRMLFiberBundleLineDisplayNode.cxx
      vtkMRMLFiberBundleNode.cxx
      vtkMRMLFiberBundleStorageNode.cxx
      vtkMRMLFiberBundleTubeDisplayNode.cxx
      vtkMRMLNRRDStorageNode.cxx
      vtkMRMLTensorVolumeNode.cxx
      vtkMRMLVectorVolumeNode.cxx        
      )
  endif(MRML_USE_vtkTeem)
endif(MRML_USE_TEEM)

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

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
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
  )

if(MRML_USE_TEEM)
  target_link_libraries(${lib_name} ${TEEM_LIBRARIES})
  if(MRML_USE_vtkTeem)
    target_link_libraries(${lib_name} vtkTeem)
  endif(MRML_USE_vtkTeem)
endif(MRML_USE_TEEM)

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
