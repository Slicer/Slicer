option(vtkSegmentationCore_USE_UUID "Use UUIDs for generating segment IDs. Requires gdcm library." ON)
mark_as_advanced(vtkSegmentationCore_USE_UUID)

if(vtkSegmentationCore_USE_UUID)
  vtk_module_find_package(
    PACKAGE ITK
    COMPONENTS
      ITKGDCM
  )
  include(${ITK_USE_FILE})
endif()

vtk_module_find_package(PACKAGE vtkAddon)

set(configure_header_file vtkSegmentationCoreConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSegmentationCoreExport.h"
  OUTPUT_VAR wrapper_header_file
)

set(classes
  vtkOrientedImageData
  vtkOrientedImageDataResample
  vtkSegment
  vtkSegmentation
  vtkSegmentationConversionParameters
  vtkSegmentationConversionPath
  vtkSegmentationConverter
  vtkSegmentationConverterFactory
  vtkSegmentationConverterRule
  vtkSegmentationHistory
  vtkSegmentationModifier
  vtkTopologicalHierarchy
  vtkBinaryLabelmapToClosedSurfaceConversionRule
  vtkClosedSurfaceToBinaryLabelmapConversionRule
  vtkCalculateOversamplingFactor
  vtkClosedSurfaceToFractionalLabelmapConversionRule
  vtkFractionalLabelmapToClosedSurfaceConversionRule
  vtkPolyDataToFractionalLabelmapFilter
)

set(headers
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  ${wrapper_header_file}
)

vtk_module_add_module(Slicer::SegmentationCore
  EXPORT_MACRO_PREFIX vtkSegmentationCore
  CLASSES ${classes}
  HEADERS ${headers}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::SegmentationCore PUBLIC vtkAddon)

if (vtkSegmentationCore_USE_UUID)
  # In order to use UUIDs for segment IDs, we need to link against the gdcmMSFF library
  # for gdcm::UIDGenerator.
  vtk_module_link(Slicer::SegmentationCore PRIVATE gdcmMSFF)
endif()

vtk_module_set_property(Slicer::SegmentationCore PROPERTY FOLDER VALUE vtkSegmentationCore)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
