
set(MRML_APPLICATION_NAME "${Slicer_MAIN_PROJECT_APPLICATION_NAME}" CACHE STRING "Current MRML scene creator application name" FORCE)

# Convert version to 0xXXYYZZ format where XX=major, YY=minor, ZZ=patch version in hexadecimal.
# Add a leading 1 (256*256*256*1) to force having a leading 0 for major version (e.g., 04 instead of just 4)
math(EXPR MRML_APPLICATION_VERSION_HEX "1 << 24
  | ${Slicer_MAIN_PROJECT_VERSION_MAJOR} << 16
  | ${Slicer_MAIN_PROJECT_VERSION_MINOR} << 8
  | ${Slicer_MAIN_PROJECT_VERSION_PATCH}"
  OUTPUT_FORMAT HEXADECIMAL)

# Remove leading 1 (remove 0x1 and add back just 0x)
string(SUBSTRING "${MRML_APPLICATION_VERSION_HEX}" 3 6 MRML_APPLICATION_VERSION_HEX)
set(MRML_APPLICATION_VERSION "0x${MRML_APPLICATION_VERSION_HEX}" CACHE STRING "Current MRML application version" FORCE)
set(MRML_APPLICATION_REVISION "${Slicer_MAIN_PROJECT_REVISION}" CACHE STRING "Current MRML application revision number" FORCE)

# MRML supports Slicer3 and Slicer4 scenes
set(MRML_APPLICATION_SUPPORT_VERSION "0x030000" CACHE STRING "MRML application version to support" FORCE)

# Used by configure file, this variable was introduced long time ago but never truely used.
# vtkTeem is currently a hard dependency of the Slicer::MRMLCore module.
set(MRML_USE_vtkTeem ON)

# Name of the environment variable that contains the application home directory.
set(MRML_APPLICATION_HOME_DIR_ENV "SLICER_HOME")

# Name of the environment variable that contains subdirectory
# within the application directory where shared files are stored.
set(MRML_APPLICATION_SHARE_SUBDIR_ENV "SLICER_SHARE_DIR")

# Name of the environment variable that contains the requested OpenGL profile.
# Accepted values are "default" (same as not specifying a value) or
# to force a specific profile: "no", "core", or  "compatibility".
set(MRML_APPLICATION_OPENGL_PROFILE_ENV "SLICER_OPENGL_PROFILE")

# These variables are referenced for test data
set(MRMLCore_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")
set(MRMLCore_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "")

message(STATUS "Configuring MRML")
message(STATUS "  MRML_APPLICATION_NAME is ${MRML_APPLICATION_NAME}")
message(STATUS "  MRML_APPLICATION_VERSION is ${MRML_APPLICATION_VERSION}")
message(STATUS "  MRML_APPLICATION_REVISION is ${MRML_APPLICATION_REVISION}")
message(STATUS "  MRML_APPLICATION_SUPPORT_VERSION is ${MRML_APPLICATION_SUPPORT_VERSION}")

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
  PRIVATE
    ${LibArchive_INCLUDE_DIRS}
    ${Teem_INCLUDE_DIRS}
)

vtk_module_set_property(Slicer::MRMLCore PROPERTY FOLDER VALUE MRMLCore)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
