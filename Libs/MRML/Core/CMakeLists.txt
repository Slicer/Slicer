set(MRML_APPLICATION_NAME "slicer-core" CACHE STRING "Current MRML scene creator application name" )
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

# Name of the environment variable that contains the application home directory.
set(MRML_APPLICATION_HOME_DIR_ENV "SLICER_HOME")

# Name of the environment variable that contains subdirectory
# within the application directory where shared files are stored.
set(MRML_APPLICATION_SHARE_SUBDIR_ENV "SLICER_SHARE_DIR")

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

vtk_module_find_package(
  PACKAGE LibArchive
  PRIVATE_IF_SHARED
)

vtk_module_find_package(
  PACKAGE RapidJSON
  PRIVATE_IF_SHARED
)

vtk_module_find_package(
  PACKAGE Teem
  PRIVATE_IF_SHARED
)

vtk_module_find_package(
  PACKAGE ITK
  COMPONENTS
    ITKCommon
    ITKIOImageBase
    ITKIONRRD
    ITKIOSpatialObjects
    ITKIOTransformBase
    ITKIOTransformHDF5
    ITKIOTransformInsightLegacy
    ITKIOTransformMatlab
    ITKMesh
    ITKTransform
)

include(${ITK_USE_FILE})

vtk_module_find_package(PACKAGE vtkAddon)

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkMRMLConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

slicer_generate_module_wrapper(
  HEADER_NAME "vtkMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
  ADDITIONAL_INCLUDES "vtkMRMLConfigure.h"
)

# vtkMRMLMarkupsFiducialStorageNode.cxx expect vtkSlicerVersionConfigureMinimal.h to define Slicer_VERSION to some string
set(version_header_file "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerVersionConfigureMinimal.h")
file(WRITE "${version_header_file}" "#define Slicer_VERSION \"slicer-core-${SKBUILD_PROJECT_VERSION}\"")

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(classes
  vtkArchive
  vtkCacheManager
  vtkCapPolyData
  vtkCodedEntry
  vtkCurveMeasurementsCalculator
  vtkDataFileFormatHelper
  vtkDataIOManager
  vtkDataTransfer
  vtkEventBroker
  vtkImageMapToWindowLevelAddon
  vtkImageMathematicsAddon
  vtkImplicitInvertableBoolean
  vtkMRMLAbstractLayoutNode
  vtkMRMLAbstractViewNode
  vtkMRMLBSplineTransformNode
  vtkMRMLCameraNode
  vtkMRMLClipModelsNode
  vtkMRMLClipNode
  vtkMRMLColorNode
  vtkMRMLColors
  vtkMRMLColorTableNode
  vtkMRMLColorTableStorageNode
  vtkMRMLCoreTestingUtilities
  vtkMRMLCrosshairNode
  vtkMRMLdGEMRICProceduralColorNode
  vtkMRMLDiffusionImageVolumeNode
  vtkMRMLDiffusionTensorDisplayPropertiesNode
  vtkMRMLDiffusionTensorVolumeDisplayNode
  vtkMRMLDiffusionTensorVolumeNode
  vtkMRMLDiffusionTensorVolumeSliceDisplayNode
  vtkMRMLDiffusionWeightedVolumeDisplayNode
  vtkMRMLDiffusionWeightedVolumeNode
  vtkMRMLDisplayableHierarchyNode
  vtkMRMLDisplayableNode
  vtkMRMLDisplayNode
  vtkMRMLFolderDisplayNode
  vtkMRMLGlyphableVolumeDisplayNode
  vtkMRMLGlyphableVolumeSliceDisplayNode
  vtkMRMLGridTransformNode
  vtkMRMLHierarchyNode
  vtkMRMLHierarchyStorageNode
  vtkMRMLI18N
  vtkMRMLInteractionEventData
  vtkMRMLInteractionNode
  vtkMRMLJsonElement
  vtkMRMLLabelMapVolumeDisplayNode
  vtkMRMLLabelMapVolumeNode
  vtkMRMLLayoutNode
  vtkMRMLLinearTransformNode
  vtkMRMLLinearTransformSequenceStorageNode
  vtkMRMLMarkupsAngleNode
  vtkMRMLMarkupsClosedCurveNode
  vtkMRMLMarkupsCurveNode
  vtkMRMLMarkupsDisplayNode
  vtkMRMLMarkupsFiducialDisplayNode
  vtkMRMLMarkupsFiducialNode
  vtkMRMLMarkupsFiducialStorageNode
  vtkMRMLMarkupsLineNode
  vtkMRMLMarkupsNode
  vtkMRMLMarkupsPlaneDisplayNode
  vtkMRMLMarkupsPlaneNode
  vtkMRMLMarkupsROIDisplayNode
  vtkMRMLMarkupsROINode
  vtkMRMLMarkupsStorageNode
  vtkMRMLMeasurement
  vtkMRMLMeasurementAngle
  vtkMRMLMeasurementArea
  vtkMRMLMeasurementLength
  vtkMRMLMeasurementVolume
  vtkMRMLMessageCollection
  vtkMRMLModelDisplayNode
  vtkMRMLModelHierarchyNode
  vtkMRMLModelNode
  vtkMRMLNodeFactory
  vtkMRMLModelStorageNode
  vtkMRMLNode
  vtkMRMLNRRDStorageNode
  vtkMRMLParser
  vtkMRMLPETProceduralColorNode
  vtkMRMLPlotChartNode
  vtkMRMLPlotSeriesNode
  vtkMRMLPlotViewNode
  vtkMRMLProceduralColorNode
  vtkMRMLProceduralColorStorageNode
  vtkMRMLROIListNode
  vtkMRMLROINode
  vtkMRMLScalarVolumeDisplayNode
  vtkMRMLScalarVolumeNode
  vtkMRMLScene
  vtkMRMLSceneViewNode
  vtkMRMLSceneViewStorageNode
  vtkMRMLScriptedModuleNode
  vtkMRMLSegmentationDisplayNode
  vtkMRMLSegmentationNode
  vtkMRMLSegmentationStorageNode
  vtkMRMLSelectionNode
  vtkMRMLSequenceNode
  vtkMRMLSequenceStorageNode
  vtkMRMLSliceCompositeNode
  vtkMRMLSliceDisplayNode
  vtkMRMLSliceNode
  vtkMRMLSnapshotClipNode
  vtkMRMLStaticMeasurement
  vtkMRMLStorableNode
  vtkMRMLStorageNode
  vtkMRMLStreamingVolumeNode
  vtkMRMLSubjectHierarchyLegacyNode
  vtkMRMLSubjectHierarchyNode
  vtkMRMLTableNode
  vtkMRMLTableSQLiteStorageNode
  vtkMRMLTableStorageNode
  vtkMRMLTableViewNode
  vtkMRMLTensorVolumeNode
  vtkMRMLTextNode
  vtkMRMLTextStorageNode
  vtkMRMLTransformableNode
  vtkMRMLTransformDisplayNode
  vtkMRMLTransformNode
  vtkMRMLTransformSequenceStorageNode
  vtkMRMLTransformStorageNode
  vtkMRMLUnitNode
  vtkMRMLVectorVolumeDisplayNode
  vtkMRMLVectorVolumeNode
  vtkMRMLViewNode
  vtkMRMLVolumeArchetypeStorageNode
  vtkMRMLVolumeDisplayNode
  vtkMRMLVolumeHeaderlessStorageNode
  vtkMRMLVolumeNode
  vtkMRMLVolumeSequenceStorageNode
  vtkObservation
  vtkObserverManager
  vtkPermissionPrompter
  vtkProjectMarkupsCurvePointsFilter
  vtkTagTable
  vtkTagTableCollection
  vtkURIHandler
)

set(headers
  vtkMRML.h
  vtkMRMLNodePropertyMacros.h
  vtkMRMLTranslator.h
  vtkMRMLSubjectHierarchyConstants.h
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  ${wrapper_header_file}
)

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

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------

vtk_module_add_module(Slicer::MRMLCore
  EXPORT_MACRO_PREFIX VTK_MRML
  CLASSES ${classes}
  HEADERS ${headers}
  PRIVATE_HEADERS ${version_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::MRMLCore
  PUBLIC
    vtkAddon
    ITKFactoryRegistration
  PRIVATE
    ${ITK_LIBRARIES}
    RapidJSON
    ${LibArchive_LIBRARY}
    ${Teem_LIBRARIES}
)

vtk_module_include(Slicer::MRMLCore
  PUBLIC
    ${ITKFactoryRegistration_INCLUDE_DIRS}
  PRIVATE
    ${LibArchive_INCLUDE_DIRS}
    ${Teem_INCLUDE_DIRS}
)

vtk_module_set_property(Slicer::MRMLCore PROPERTY FOLDER VALUE MRMLCore)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
