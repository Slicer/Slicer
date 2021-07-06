project(MRMLCore)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.13.4)
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
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
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
  vtkCodedEntry.cxx
  vtkEventBroker.cxx
  vtkDataFileFormatHelper.cxx
  vtkMRMLMeasurement.cxx
  vtkMRMLStaticMeasurement.cxx
  vtkMRMLLogic.cxx
  vtkMRMLAbstractLayoutNode.cxx
  vtkMRMLAbstractViewNode.cxx
  vtkMRMLCameraNode.cxx
  vtkMRMLClipModelsNode.cxx
  vtkMRMLColorNode.cxx
  vtkMRMLColors.cxx
  vtkMRMLColorTableNode.cxx
  vtkMRMLColorTableStorageNode.cxx
  vtkMRMLCoreTestingUtilities.cxx
  vtkMRMLCrosshairNode.cxx
  vtkMRMLDiffusionTensorDisplayPropertiesNode.cxx
  vtkMRMLDiffusionWeightedVolumeDisplayNode.cxx
  vtkMRMLDiffusionWeightedVolumeNode.cxx
  vtkMRMLDisplayNode.cxx
  vtkMRMLDisplayableNode.cxx
  vtkMRMLFolderDisplayNode.cxx
  vtkMRMLHierarchyNode.cxx
  vtkMRMLHierarchyStorageNode.cxx
  vtkMRMLDisplayableHierarchyNode.cxx
  vtkMRMLInteractionNode.cxx
  vtkMRMLLabelMapVolumeDisplayNode.cxx
  vtkMRMLLabelMapVolumeNode.cxx
  vtkMRMLLinearTransformNode.cxx
  vtkMRMLLinearTransformSequenceStorageNode.cxx
  vtkMRMLLinearTransformSequenceStorageNode.h
  vtkMRMLMarkupsStorageNode.cxx
  vtkMRMLMessageCollection.cxx
  vtkMRMLModelDisplayNode.cxx
  vtkMRMLModelHierarchyNode.cxx
  vtkMRMLModelNode.cxx
  vtkMRMLModelStorageNode.cxx
  vtkMRMLNode.cxx
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
  vtkMRMLSegmentationNode.cxx
  vtkMRMLSegmentationNode.h
  vtkMRMLSegmentationDisplayNode.cxx
  vtkMRMLSegmentationDisplayNode.h
  vtkMRMLSegmentationStorageNode.cxx
  vtkMRMLSegmentationStorageNode.h
  vtkMRMLSequenceNode.cxx
  vtkMRMLSequenceNode.h
  vtkMRMLSequenceStorageNode.cxx
  vtkMRMLSequenceStorageNode.h
  vtkMRMLSelectionNode.cxx
  vtkMRMLSliceCompositeNode.cxx
  vtkMRMLSliceDisplayNode.cxx
  vtkMRMLSliceNode.cxx
  vtkMRMLSnapshotClipNode.cxx
  vtkMRMLStorableNode.cxx
  vtkMRMLStorageNode.cxx
  vtkMRMLSubjectHierarchyConstants.h
  vtkMRMLSubjectHierarchyNode.cxx
  vtkMRMLSubjectHierarchyNode.h
  vtkMRMLSubjectHierarchyLegacyNode.cxx
  vtkMRMLSubjectHierarchyLegacyNode.h
  vtkMRMLTableNode.cxx
  vtkMRMLTableStorageNode.cxx
  vtkMRMLTableSQLiteStorageNode.cxx
  vtkMRMLTableViewNode.cxx
  vtkMRMLTextNode.cxx
  vtkMRMLTextStorageNode.cxx
  vtkMRMLTransformNode.cxx
  vtkMRMLTransformStorageNode.cxx
  vtkMRMLTransformDisplayNode.cxx
  vtkMRMLTransformableNode.cxx
  vtkMRMLUnitNode.cxx
  vtkMRMLVectorVolumeDisplayNode.cxx
  vtkMRMLViewNode.cxx
  vtkMRMLVolumeArchetypeStorageNode.cxx
  vtkMRMLVolumeDisplayNode.cxx
  vtkMRMLGlyphableVolumeDisplayNode.cxx
  vtkMRMLGlyphableVolumeSliceDisplayNode.cxx
  vtkMRMLVolumeHeaderlessStorageNode.cxx
  vtkMRMLVolumeNode.cxx
  vtkMRMLVolumeSequenceStorageNode.cxx
  vtkMRMLVolumeSequenceStorageNode.h
  vtkObservation.cxx
  vtkObserverManager.cxx
  vtkMRMLLayoutNode.cxx
  # Classes for remote data handling:
  vtkCacheManager.cxx
  vtkDataIOManager.cxx
  vtkDataTransfer.cxx
  vtkMRMLBSplineTransformNode.cxx
  vtkMRMLGridTransformNode.cxx
  vtkPermissionPrompter.cxx
  vtkURIHandler.cxx
  vtkTagTableCollection.cxx
  vtkTagTable.cxx
  vtkMRMLdGEMRICProceduralColorNode.cxx
  vtkMRMLPETProceduralColorNode.cxx
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
    vtkMRMLTensorVolumeNode.cxx
    vtkMRMLVectorVolumeNode.cxx
    vtkMRMLStreamingVolumeNode.cxx
    )
endif()

set_source_files_properties(
  vtkMRMLCoreTestingUtilities.cxx
  WRAP_EXCLUDE
  )

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
  ${VTK_TARGET_PREFIX}IOInfovis
  ${LibArchive_LIBRARY}
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
  if(${VTK_VERSION} VERSION_LESS "8.90")
    export(TARGETS ${lib_name}PythonD APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})
  endif()
  # Folder
  if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
    set_target_properties(${lib_name}Python PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    if(${VTK_VERSION} VERSION_LESS "8.90")
      set_target_properties(${lib_name}PythonD PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    endif()
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
