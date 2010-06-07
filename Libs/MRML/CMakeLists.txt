project(MRML)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

option(MRML_USE_Teem "Build MRML with Teem support." ON)
mark_as_advanced(MRML_USE_Teem)
if(MRML_USE_Teem)
  find_package(Teem REQUIRED)
  include(${Teem_USE_FILE})
  option(MRML_USE_vtkTeem "Build MRML with vtkTeem support." ON)
  mark_as_advanced(MRML_USE_vtkTeem)
endif(MRML_USE_Teem)

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

if(MRML_USE_Teem)
  set(packages ${packages} 
    vtkTeem
    )
endif(MRML_USE_Teem)
  
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
  vtkDataFileFormatHelper.cxx
  vtkMRMLLogic.cxx
  vtkMRMLCameraNode.cxx
  vtkMRMLClipModelsNode.cxx
  vtkMRMLColorNode.cxx
  vtkMRMLColorTableNode.cxx
  vtkMRMLColorTableStorageNode.cxx
  vtkMRMLCrosshairNode.cxx
  vtkMRMLDiffusionTensorDisplayPropertiesNode.cxx
  vtkMRMLDiffusionWeightedVolumeDisplayNode.cxx
  vtkMRMLDiffusionWeightedVolumeNode.cxx
  vtkMRMLDisplayNode.cxx
  vtkMRMLDisplayableNode.cxx
  vtkMRMLFiberBundleDisplayNode.cxx
  vtkMRMLFiducial.cxx
  vtkMRMLFiducialListNode.cxx
  vtkMRMLFiducialListStorageNode.cxx
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
  vtkMRMLGlyphableVolumeDisplayNode.cxx
  vtkMRMLGlyphableVolumeSliceDisplayNode.cxx
  vtkMRMLGlyphableVolumeDisplayPropertiesNode.cxx
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
  vtkTagTableCollection.cxx
  vtkTagTable.cxx
  vtkMRMLVolumePropertyNode.cxx
  vtkMRMLVolumePropertyStorageNode.cxx
  vtkMRMLdGEMRICProceduralColorNode.cxx
  vtkMRMLPETProceduralColorNode.cxx
  # Classes for 2D Plotting
  vtkMRMLArrayPlotNode.cxx
  vtkMRMLXYPlotManagerNode.cxx
  vtkMRMLDoubleArrayNode.cxx
  vtkMRMLPlotNode.cxx
  vtkMRMLOrthogonalLinePlotNode.cxx
  #
  vtkMRMLDoubleArrayStorageNode.cxx
  # Note: when you add new node types, add
  # register code to the vtkMRMLScene constructor
  )

if(MRML_USE_Teem)
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
endif(MRML_USE_Teem)

# Abstract/pure virtual classes

set_source_files_properties(
  vtkMRMLNode.cxx 
  vtkMRMLVolumeNode.cxx
  vtkMRMLTransformNode.cxx
  vtkMRMLDisplayNode.cxx
  vtkMRMLDisplayableNode.cxx
  vtkMRMLVolumeDisplayNode.cxx
  vtkMRMLFiberBundleDisplayNode.cxx
  ABSTRACT
  )

# --------------------------------------------------------------------------
# Wrapping
IF (VTK_WRAP_TCL)
  include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  set(VTK_WRAP_HINTS ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping/Tcl/hints)
  vtk_wrap_tcl3(MRML MRML_TCL_SRCS "${MRML_SRCS}" "")
ENDIF (VTK_WRAP_TCL)

# --------------------------------------------------------------------------
# Build and install the library

SET(srcs ${MRML_SRCS})

IF (VTK_WRAP_TCL)
  SET(srcs ${srcs} ${MRML_TCL_SRCS})
ENDIF (VTK_WRAP_TCL)

SET(lib_name MRML)
ADD_LIBRARY(${lib_name} ${srcs})

SET(VTK_LIBRARIES
  vtkHybrid
  vtkRendering
  )

SET(libs
  vtkITK
  ITKAlgorithms
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  FreeSurfer
  )
  
IF(MRML_USE_vtkTeem)
  SET(libs ${libs} vtkTeem)
ENDIF(MRML_USE_vtkTeem)

TARGET_LINK_LIBRARIES(${lib_name} ${libs})


FOREACH(c ${VTK_LIBRARIES})
  SET(tcl_wrap)
  IF (VTK_WRAP_TCL)
    SET(tcl_wrap TCL)
  ENDIF()
  TARGET_LINK_LIBRARIES(${lib_name} ${c}${tcl_wrap})
ENDFOREACH(c)

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Create Python wrappers if VTK is wrapped in Python
#

IF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

  SET(KIT_PYTHON_LIBRARIES ${VTK_LIBRARIES})

  INCLUDE(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
  VTK_WRAP_PYTHON3(${lib_name}Python PYTHON_SRCS "${MRML_SRCS}")
  ADD_LIBRARY(${lib_name}PythonD ${PYTHON_SRCS})
  ADD_LIBRARY(${lib_name}Python MODULE ${lib_name}PythonInit.cxx)
  
  TARGET_LINK_LIBRARIES(${lib_name}PythonD ${lib_name})
  
  FOREACH(c ${KIT_PYTHON_LIBRARIES})
    TARGET_LINK_LIBRARIES(${lib_name}PythonD ${c}PythonD)
  ENDFOREACH(c)

  TARGET_LINK_LIBRARIES(${lib_name}Python ${lib_name}PythonD)

  ## Python modules on Windows must have the extension ".pyd"
  IF(WIN32 AND NOT CYGWIN)
    SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES SUFFIX ".pyd")
  ENDIF(WIN32 AND NOT CYGWIN)

  INSTALL(TARGETS ${lib_name}PythonD ${lib_name}Python
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development)

ENDIF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
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
