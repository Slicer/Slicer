# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------
if(NOT DEFINED VTKITK_BUILD_DICOM_SUPPORT)
  option(VTKITK_BUILD_DICOM_SUPPORT "Build with DICOM support" ON)
endif()
# DICOM files are read using GDCM. DCMTK can be used as well, if it is available in ITK (ITKIODCMTK module).
if(NOT DEFINED VTKITK_BUILD_DCMTK_SUPPORT)
  option(VTKITK_BUILD_DCMTK_SUPPORT "Build with support for reading DICOM files using DCMTK" ${VTKITK_BUILD_DICOM_SUPPORT})
endif()
if(VTKITK_BUILD_DCMTK_SUPPORT AND NOT VTKITK_BUILD_DICOM_SUPPORT)
  message(FATAL_ERROR "VTKITK_BUILD_DCMTK_SUPPORT requires VTKITK_BUILD_DICOM_SUPPORT")
endif()

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

vtk_module_find_package(
  PACKAGE ITK
  COMPONENTS
    GrowCut
    ITKAnisotropicSmoothing
    ITKCommon
    ITKConnectedComponents
    ITKDistanceMap
    ITKIODCMTK
    ITKIOGDCM
    ITKIOGE
    ITKIOImageBase
    ITKIOMeta
    ITKIONIFTI
    ITKIONRRD
    ITKImageCompose
    ITKImageFilterBase
    ITKImageFunction
    ITKImageGrid
    ITKLabelMap
    ITKGDCM
    ITKPath
    ITKRegionGrowing
    ITKThresholding
    ITKVTK
    ITKVtkGlue
    MorphologicalContourInterpolation
)
include(${ITK_USE_FILE})

vtk_module_find_package(PACKAGE vtkAddon)

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkITKConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

slicer_generate_module_wrapper(
  HEADER_NAME "vtkITKExport.h"
  OUTPUT_VAR wrapper_header_file
  ADDITIONAL_INCLUDES "vtkITKConfigure.h"
)

set(classes
  vtkITKArchetypeDiffusionTensorImageReaderFile
  vtkITKArchetypeImageSeriesReader
  vtkITKArchetypeImageSeriesScalarReader
  vtkITKArchetypeImageSeriesVectorReaderFile
  vtkITKArchetypeImageSeriesVectorReaderSeries
  vtkITKImageThresholdCalculator
  vtkITKImageWriter
  vtkITKGradientAnisotropicDiffusionImageFilter
  vtkITKGrowCut
  vtkITKDistanceTransform
  vtkITKLabelShapeStatistics
  vtkITKLevelTracingImageFilter
  vtkITKLevelTracing3DImageFilter
  vtkITKIslandMath
  vtkITKImageMargin
  vtkITKMorphologicalContourInterpolator
  vtkITKImageSequenceReader
  vtkITKImageSequenceWriter
)

set(sources
  vtkITKNumericTraits.cxx
  vtkITKImageToImageFilter.h # put as source to ensure their symbols are defined
  vtkITKImageToImageFilterFF.h
  vtkITKImageToImageFilter2DFF.h
  vtkITKImageToImageFilterFUL.h
  vtkITKImageToImageFilterSS.h
  vtkITKImageToImageFilterULUL.h
  vtkITKImageToImageFilterUSUL.h
  vtkITKImageToImageFilterUSUS.h
  vtkITKImageToImageFilterUSF.h
)

set(headers
  vtkITKNumericTraits.h
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  ${wrapper_header_file}
)

vtk_module_add_module(Slicer::ITK
  EXPORT_MACRO_PREFIX VTK_ITK
  CLASSES ${classes}
  SOURCES ${sources}
  HEADERS ${headers}
  EXCLUDE_HEADER_TEST
)

set_source_files_properties(
  vtkITKNumericTraits.cxx
  WRAP_EXCLUDE
)

vtk_module_link(Slicer::ITK PUBLIC ${ITK_LIBRARIES} vtkAddon)

vtk_module_set_property(Slicer::ITK PROPERTY FOLDER VALUE vtkITK)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
