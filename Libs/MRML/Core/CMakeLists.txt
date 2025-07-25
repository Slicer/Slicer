project(MRMLCore)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20.6...3.22.6 FATAL_ERROR)
#-----------------------------------------------------------------------------

# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------
if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

set(MRML_APPLICATION_NAME "Slicer" CACHE STRING "Current MRML scene creator application name" )
mark_as_advanced(MRML_APPLICATION_NAME)

# The format is: 0xXXYYZZ where XX is the major version, YY the minor and
# ZZ the patch version.
# This format is chosen to allow easy comparison in C++ precompiler macros.
set(MRML_APPLICATION_VERSION 0x040000 CACHE STRING "Current MRML application version" )
mark_as_advanced(MRML_APPLICATION_VERSION)

# Revision can be used to identify version with finer granularity than version.
# It is an integer number that is incremented with each new software revision.
set(MRML_APPLICATION_REVISION 0 CACHE STRING "Current MRML application revision number" )
mark_as_advanced(MRML_APPLICATION_REVISION)

# MRML scenes that are older than MRML_APPLICATION_SUPPORT_VERSION won't be supported.
# Or said differently, MRML_APPLICATION_SUPPORT_VERSION is the earliest version supported
# for a MRML scene.
# The format is: 0xXXYYZZ where XX is the major version, YY the minor and
# ZZ the patch version.
# For example, set MRML_APPLICATION_SUPPORT_VERSION to 0x040000 if you don't want to
# support scenes of Slicer 3.
# Supports Slicer3 and Slicer4 scenes by default.
set(MRML_APPLICATION_SUPPORT_VERSION 0x030000 CACHE STRING "MRML application version to support" )
mark_as_advanced(MRML_APPLICATION_SUPPORT_VERSION)

option(MRML_USE_vtkTeem "Build MRML with vtkTeem support." ON)
mark_as_advanced(MRML_USE_vtkTeem)

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# VTK
#
#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

#
# ITK
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ITKCommon
  ITKIOImageBase
  ITKIOSpatialObjects
  ITKIOTransformBase
  ITKMesh
  ITKTransform
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
if(ITK_VERSION VERSION_GREATER_EQUAL "5.3")
  foreach(factory_uc IN ITEMS "IMAGEIO" "MESHIO" "TRANSFORMIO")
    set(ITK_NO_${factory_uc}_FACTORY_REGISTER_MANAGER 1)
  endforeach()
else()
  set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
endif()
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS ${ITKFactoryRegistration_INCLUDE_DIRS})
include(${ITK_USE_FILE})

#
# LibArchive
#
find_package(LibArchive REQUIRED MODULE)

#
# vtkTeem
#
if(MRML_USE_vtkTeem)
  find_package(Teem REQUIRED)
  include(${Teem_USE_FILE})
endif()

#
# vtkAddon
#
if(NOT Slicer_BUILD_vtkAddon)
  find_package(vtkAddon REQUIRED)
endif()

#
# RapidJSON
#
find_package(RapidJSON REQUIRED)

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${vtkAddon_INCLUDE_DIRS}
  ${vtkITK_INCLUDE_DIRS}
  ${vtkSegmentationCore_INCLUDE_DIRS}
  ${LibArchive_INCLUDE_DIR}
  )
if(MRML_USE_vtkTeem)
  list(APPEND include_dirs ${vtkTeem_INCLUDE_DIRS})
endif()
include_directories(BEFORE ${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkMRMLConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(MRMLCore_SRCS
  vtkArchive.cxx
  vtkArchive.h
  vtkCacheManager.cxx
  vtkCapPolyData.cxx
  vtkCodedEntry.cxx
  vtkCurveMeasurementsCalculator.cxx
  vtkCurveMeasurementsCalculator.h
  vtkDataFileFormatHelper.cxx
  vtkDataIOManager.cxx
  vtkDataTransfer.cxx
  vtkEventBroker.cxx
  vtkImageMathematicsAddon.cxx
  vtkImplicitInvertableBoolean.cxx
  vtkMRMLAbstractLayoutNode.cxx
  vtkMRMLAbstractViewNode.cxx
  vtkMRMLBSplineTransformNode.cxx
  vtkMRMLCameraNode.cxx
  vtkMRMLClipModelsNode.cxx
  vtkMRMLClipNode.cxx
  vtkMRMLColorNode.cxx
  vtkMRMLColorTableNode.cxx
  vtkMRMLColorTableStorageNode.cxx
  vtkMRMLColors.cxx
  vtkMRMLCoreTestingUtilities.cxx
  vtkMRMLCrosshairNode.cxx
  vtkMRMLDiffusionTensorDisplayPropertiesNode.cxx
  vtkMRMLDiffusionWeightedVolumeDisplayNode.cxx
  vtkMRMLDiffusionWeightedVolumeNode.cxx
  vtkMRMLDisplayNode.cxx
  vtkMRMLDisplayableHierarchyNode.cxx
  vtkMRMLDisplayableNode.cxx
  vtkMRMLFolderDisplayNode.cxx
  vtkMRMLGlyphableVolumeDisplayNode.cxx
  vtkMRMLGlyphableVolumeSliceDisplayNode.cxx
  vtkMRMLGridTransformNode.cxx
  vtkMRMLHierarchyNode.cxx
  vtkMRMLHierarchyStorageNode.cxx
  vtkMRMLI18N.cxx
  vtkMRMLI18N.h
  vtkMRMLInteractionEventData.cxx
  vtkMRMLInteractionEventData.h
  vtkMRMLInteractionNode.cxx
  vtkMRMLJsonElement.cxx
  vtkMRMLLabelMapVolumeDisplayNode.cxx
  vtkMRMLLabelMapVolumeNode.cxx
  vtkMRMLLayoutNode.cxx
  vtkMRMLLinearTransformNode.cxx
  vtkMRMLLinearTransformSequenceStorageNode.cxx
  vtkMRMLLinearTransformSequenceStorageNode.h
  vtkMRMLLogic.cxx
  vtkMRMLMarkupsAngleNode.cxx
  vtkMRMLMarkupsAngleNode.h
  vtkMRMLMarkupsClosedCurveNode.cxx
  vtkMRMLMarkupsClosedCurveNode.h
  vtkMRMLMarkupsCurveNode.cxx
  vtkMRMLMarkupsCurveNode.h
  vtkMRMLMarkupsDisplayNode.cxx
  vtkMRMLMarkupsDisplayNode.h
  vtkMRMLMarkupsFiducialDisplayNode.cxx
  vtkMRMLMarkupsFiducialDisplayNode.h
  vtkMRMLMarkupsFiducialNode.cxx
  vtkMRMLMarkupsFiducialNode.h
  vtkMRMLMarkupsFiducialStorageNode.cxx
  vtkMRMLMarkupsFiducialStorageNode.h
  vtkMRMLMarkupsLineNode.cxx
  vtkMRMLMarkupsLineNode.h
  vtkMRMLMarkupsNode.cxx
  vtkMRMLMarkupsNode.h
  vtkMRMLMarkupsPlaneDisplayNode.cxx
  vtkMRMLMarkupsPlaneDisplayNode.h
  vtkMRMLMarkupsPlaneNode.cxx
  vtkMRMLMarkupsPlaneNode.h
  vtkMRMLMarkupsROIDisplayNode.cxx
  vtkMRMLMarkupsROIDisplayNode.h
  vtkMRMLMarkupsROINode.cxx
  vtkMRMLMarkupsROINode.h
  vtkMRMLMarkupsStorageNode.cxx
  vtkMRMLMeasurement.cxx
  vtkMRMLMeasurementAngle.cxx
  vtkMRMLMeasurementAngle.h
  vtkMRMLMeasurementArea.cxx
  vtkMRMLMeasurementArea.h
  vtkMRMLMeasurementLength.cxx
  vtkMRMLMeasurementLength.h
  vtkMRMLMeasurementVolume.cxx
  vtkMRMLMeasurementVolume.h
  vtkMRMLMessageCollection.cxx
  vtkMRMLModelDisplayNode.cxx
  vtkMRMLModelHierarchyNode.cxx
  vtkMRMLModelNode.cxx
  vtkMRMLModelStorageNode.cxx
  vtkMRMLNode.cxx
  vtkMRMLPETProceduralColorNode.cxx
  vtkMRMLParser.cxx
  vtkMRMLPlotChartNode.cxx
  vtkMRMLPlotSeriesNode.cxx
  vtkMRMLPlotViewNode.cxx
  vtkMRMLProceduralColorNode.cxx
  vtkMRMLProceduralColorStorageNode.cxx
  vtkMRMLROIListNode.cxx
  vtkMRMLROINode.cxx
  vtkMRMLScalarVolumeDisplayNode.cxx
  vtkMRMLScalarVolumeNode.cxx
  vtkMRMLScene.cxx
  vtkMRMLSceneViewNode.cxx
  vtkMRMLSceneViewStorageNode.cxx
  vtkMRMLScriptedModuleNode.cxx
  vtkMRMLScriptedModuleNode.h
  vtkMRMLSegmentationDisplayNode.cxx
  vtkMRMLSegmentationDisplayNode.h
  vtkMRMLSegmentationNode.cxx
  vtkMRMLSegmentationNode.h
  vtkMRMLSegmentationStorageNode.cxx
  vtkMRMLSegmentationStorageNode.h
  vtkMRMLSelectionNode.cxx
  vtkMRMLSequenceNode.cxx
  vtkMRMLSequenceNode.h
  vtkMRMLSequenceStorageNode.cxx
  vtkMRMLSequenceStorageNode.h
  vtkMRMLSliceCompositeNode.cxx
  vtkMRMLSliceDisplayNode.cxx
  vtkMRMLSliceNode.cxx
  vtkMRMLSnapshotClipNode.cxx
  vtkMRMLStaticMeasurement.cxx
  vtkMRMLStorableNode.cxx
  vtkMRMLStorageNode.cxx
  vtkMRMLSubjectHierarchyConstants.h
  vtkMRMLSubjectHierarchyLegacyNode.cxx
  vtkMRMLSubjectHierarchyLegacyNode.h
  vtkMRMLSubjectHierarchyNode.cxx
  vtkMRMLSubjectHierarchyNode.h
  vtkMRMLTableNode.cxx
  vtkMRMLTableSQLiteStorageNode.cxx
  vtkMRMLTableStorageNode.cxx
  vtkMRMLTableViewNode.cxx
  vtkMRMLTextNode.cxx
  vtkMRMLTextStorageNode.cxx
  vtkMRMLTransformDisplayNode.cxx
  vtkMRMLTransformNode.cxx
  vtkMRMLTransformStorageNode.cxx
  vtkMRMLTransformableNode.cxx
  vtkMRMLTranslator.h
  vtkMRMLUnitNode.cxx
  vtkMRMLVectorVolumeDisplayNode.cxx
  vtkMRMLViewNode.cxx
  vtkMRMLVolumeArchetypeStorageNode.cxx
  vtkMRMLVolumeDisplayNode.cxx
  vtkMRMLVolumeHeaderlessStorageNode.cxx
  vtkMRMLVolumeNode.cxx
  vtkMRMLVolumeSequenceStorageNode.cxx
  vtkMRMLVolumeSequenceStorageNode.h
  vtkMRMLdGEMRICProceduralColorNode.cxx
  vtkObservation.cxx
  vtkObserverManager.cxx
  vtkPermissionPrompter.cxx
  vtkProjectMarkupsCurvePointsFilter.cxx
  vtkProjectMarkupsCurvePointsFilter.h
  vtkTagTable.cxx
  vtkTagTableCollection.cxx
  vtkURIHandler.cxx
  # Note: when you add new node types, add
  # register code to the vtkMRMLScene constructor
  )

if(MRML_USE_vtkTeem)
  set(MRMLCore_SRCS
    ${MRMLCore_SRCS}
    vtkMRMLDiffusionImageVolumeNode.cxx
    vtkMRMLDiffusionTensorVolumeDisplayNode.cxx
    vtkMRMLDiffusionTensorVolumeNode.cxx
    vtkMRMLDiffusionTensorVolumeSliceDisplayNode.cxx
    vtkMRMLNRRDStorageNode.cxx
    vtkMRMLStreamingVolumeNode.cxx
    vtkMRMLTensorVolumeNode.cxx
    vtkMRMLVectorVolumeNode.cxx
    )
endif()

set_source_files_properties(
  vtkMRMLCoreTestingUtilities.cxx
  WRAP_EXCLUDE
  )

# Abstract/pure virtual classes

set_source_files_properties(
  vtkMRMLDisplayNode.cxx
  vtkMRMLDisplayableNode.cxx
  vtkMRMLNode.cxx
  vtkMRMLTransformNode.cxx
  vtkMRMLVolumeDisplayNode.cxx
  vtkMRMLVolumeNode.cxx
  ABSTRACT
  )

string(REGEX REPLACE "\\.cxx" ".h" MRMLCore_SRCS_HEADERS "${MRMLCore_SRCS}")
source_group("Header Files" FILES ${MRMLCore_SRCS_HEADERS})

#-----------------------------------------------------------------------------
# Wrapping hints used by all Tcl, Python and Java wrapping.
# It can also be used by external wrapping tools.
set(VTK_WRAP_HINTS ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping/Tcl/hints)
# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(srcs ${MRMLCore_SRCS} ${MRMLCore_SRCS_HEADERS})

set(lib_name ${PROJECT_NAME})
add_library(${lib_name} ${srcs})

set(libs
  vtkAddon
  vtkITK
  vtkSegmentationCore
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  VTK::IOInfovis
  ${LibArchive_LIBRARY}
  RapidJSON
  )
if(MRML_USE_vtkTeem)
  list(APPEND libs vtkTeem)
endif()
target_link_libraries(${lib_name} ${libs})

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Folder
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_FOLDER)
  set(${PROJECT_NAME}_FOLDER ${PROJECT_NAME})
endif()
if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
  set_target_properties(${lib_name} PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
if(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${MRMLCore_SRCS}
    KIT_INSTALL_BIN_DIR ${${PROJECT_NAME}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${PROJECT_NAME}_INSTALL_LIB_DIR}
    )
  # Export target
  export(TARGETS ${lib_name}Python APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})
  # Folder
  if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
    set_target_properties(${lib_name}Python PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    if(TARGET ${lib_name}Hierarchy)
      set_target_properties(${lib_name}Hierarchy PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    endif()
  endif()
endif()

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Install Test Data
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  #file(GLOB_RECURSE MRMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.mrml")
  #file(GLOB_RECURSE TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.tcl")
  #file(GLOB_RECURSE XMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.xml")
  #file(GLOB_RECURSE GZFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.gz")
  #file(GLOB_RECURSE NHDRFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.nhdr")
  #file(GLOB_RECURSE NRRDFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.nrrd")
  #file(GLOB_RECURSE VTKFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.vtk")

  #foreach(file
  #    ${MRMLFILES}
  #    ${TCLFILES}
  #    ${XMLFILES}
  #    ${GZFILES}
  #    ${NHDRFILES}
  #    ${NRRDFILES}
  #    ${VTKFILES}
  #    )
  #  message(file:${file})
  #  configure_file(
  #    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
  #    ${CMAKE_BINARY_DIR}/share/${PROJECT_NAME}/${file}
  #    COPYONLY
  #    )
  #endforeach()

  install(DIRECTORY
    Testing
    DESTINATION share/${PROJECT_NAME} COMPONENT Development
    PATTERN "*.cxx" EXCLUDE
    PATTERN ".svn" EXCLUDE
    PATTERN "*.tcl" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE
    )
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${vtkAddon_INCLUDE_DIRS} ${vtkSegmentationCore_INCLUDE_DIRS}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
