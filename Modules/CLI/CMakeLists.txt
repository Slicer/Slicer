
#-----------------------------------------------------------------------------
# WARNING: CLI modules should be topologically ordered.
#-----------------------------------------------------------------------------
set(cli_modules
  ACPCTransform
  AddScalarVolumes
#  BlobDetection
  CastScalarVolume
  CheckerBoardFilter
#  ConnectedComponent
  CurvatureAnisotropicDiffusion
  ExecutionModelTour
  ExtractSkeleton
  FreesurferSurfaceSectionExtraction
  GaussianBlurImageFilter
  GradientAnisotropicDiffusion
  GrayscaleFillHoleImageFilter
  GrayscaleGrindPeakImageFilter
  GrayscaleModelMaker
  HistogramMatching
  ImageLabelCombine
  LabelMapSmoothing
  MaskScalarVolume
  MedianImageFilter
  MergeModels
  ModelMaker
  ModelToLabelMap
  MultiplyScalarVolumes
  N4ITKBiasFieldCorrection
  OrientScalarVolume
  ProbeVolumeWithModel
  ResampleDTIVolume # Needed by ResampleScalarVectorDWIVolume
  ResampleScalarVectorDWIVolume # Depends on DiffusionApplications, needed by CropVolume
  RobustStatisticsSegmenter
  SimpleRegionGrowingSegmentation
  SubtractScalarVolumes
  ThresholdScalarVolume
  VotingBinaryHoleFillingImageFilter
  )
if(BUILD_TESTING)
  list(APPEND cli_modules ROITest)
endif()

if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND cli_modules CreateDICOMSeries)
  list(APPEND cli_modules PETStandardUptakeValueComputation)
endif()

if(Slicer_BUILD_DIFFUSION_SUPPORT)
  list(APPEND cli_modules
    DiffusionTensorScalarMeasurements
    DiffusionWeightedVolumeMasking
    DWIJointRicianLMMSEFilter
    DWIRicianLMMSEFilter
    DWIToDTIEstimation
    DWIUnbiasedNonLocalMeansFilter
    DTIImportExport
    TractographyLabelMapSeeding
    FiberBundleLabelSelect
    FiberTractMeasurements
    )
endif()

if(Slicer_BUILD_LEGACY_CLI)
  set(legacy_cli_modules
    BSplineToDeformationField
    ExpertAutomatedRegistration
    FiducialRegistration
    ResampleScalarVolume
    )
  if(BUILD_TESTING)
    list(APPEND legacy_cli_modules
      DiffusionTensorTest
      TestGridTransformRegistration
      )
  endif()
  list(APPEND cli_modules ${legacy_cli_modules})
endif()

if (Slicer_BUILD_EMSegment)
  ## EMSegment requires some LEGACY_CLI to work. Add them if not already added
  if(NOT Slicer_BUILD_LEGACY_CLI)
    list(APPEND cli_modules BSplineToDeformationField)
  endif()
endif()

if(BUILD_TESTING)
  # TODO Since the following snippet have been moved from CLI/Testing/CMakeLists.txt into
  # CLI/CMakeLists.txt, the variable SLICER_BRAINWEB_DATA_ROOT won't be set properly.
  # Configure the default SLICER_BRAINWEB_DATA_ROOT for the location of BrainWeb Data.
  # When this data is available, additional 3D tests are enabled.
  find_path(SLICER_BRAINWEB_DATA_ROOT brainweb165a10f17.mha
    DOC "Directory with data taken from http://public.kitware.com/pub/itk/Data/BrainWeb/")
  mark_as_advanced(SLICER_BRAINWEB_DATA_ROOT)

  set(BASELINE "${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI")
  set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
  set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Core/Testing/TestData")
  set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

  set(MRML_DATA "${Slicer_HOME}/share/MRML/Testing/TestData")
endif()

foreach(module ${cli_modules})
  list(FIND Slicer_CLIMODULES_DISABLED ${module} _module_disabled)
  set(${module}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${module})
  if(${_module_disabled} STREQUAL "-1")
    add_subdirectory(${module})
  endif()
endforeach(module)

# External modules have been moved to Modules/Remote/CMakeLists.txt
