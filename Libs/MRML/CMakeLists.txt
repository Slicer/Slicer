project(MRML)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

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
endif()

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )
if(MRML_USE_Teem)
  list(APPEND include_dirs 
    ${vtkTeem_SOURCE_DIR}
    ${vtkTeem_BINARY_DIR}
    )
endif()
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
# --------------------------------------------------------------------------
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
  vtkMRMLDisplayableHierarchyNode.cxx
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
  vtkMRMLSceneViewNode.cxx
  vtkMRMLScriptedModuleNode.cxx
  vtkMRMLScriptedModuleNode.h
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

STRING(REGEX REPLACE "\\.cxx" ".h" MRML_SRCS_HEADERS "${MRML_SRCS}")
SOURCE_GROUP("Header Files" FILES ${MRML_SRCS_HEADERS})

#-----------------------------------------------------------------------------
# Wrapping hints used by all Tcl, Python and Java wrapping.
# It can also be used by external wrapping tools.
SET(VTK_WRAP_HINTS ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping/Tcl/hints)
# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
SET(srcs ${MRML_SRCS} ${MRML_SRCS_HEADERS})

SET(lib_name ${PROJECT_NAME})
ADD_LIBRARY(${lib_name} ${srcs})

SET(libs
  vtkITK
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  FreeSurfer
  )
IF(MRML_USE_vtkTeem)
  LIST(APPEND libs vtkTeem)
ENDIF()
TARGET_LINK_LIBRARIES(${lib_name} ${libs})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF()

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  SET(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
ENDIF()
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  SET(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
ENDIF()

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${lib_name}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${lib_name}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${lib_name}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
IF(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${MRML_SRCS}
    KIT_INSTALL_BIN_DIR ${${lib_name}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${lib_name}_INSTALL_LIB_DIR}
    )
ENDIF()

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
