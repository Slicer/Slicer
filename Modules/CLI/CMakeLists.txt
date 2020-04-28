
#-----------------------------------------------------------------------------
# WARNING: CLI modules should be topologically ordered.
#-----------------------------------------------------------------------------
set(cli_modules
  ACPCTransform
  AddScalarVolumes
  CastScalarVolume
  CheckerBoardFilter
  CurvatureAnisotropicDiffusion
  ExecutionModelTour
  ExtractSkeleton
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
  ResampleDTIVolume # Needed by ResampleScalarVectorDWIVolume (compiletime dependency)
  ResampleScalarVectorDWIVolume # Depends on DiffusionApplications, needed by CropVolume (runtime dependency)
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

if(Slicer_BUILD_LEGACY_CLI)
  set(legacy_cli_modules
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

if(BUILD_TESTING)
  # TODO Since the following snippet have been moved from CLI/Testing/CMakeLists.txt into
  # CLI/CMakeLists.txt, the variable SLICER_BRAINWEB_DATA_ROOT won't be set properly.
  # Configure the default SLICER_BRAINWEB_DATA_ROOT for the location of BrainWeb Data.
  # When this data is available, additional 3D tests are enabled.
  find_path(SLICER_BRAINWEB_DATA_ROOT brainweb165a10f17.mha
    DOC "Directory with data taken from http://public.kitware.com/pub/itk/Data/BrainWeb/")
  mark_as_advanced(SLICER_BRAINWEB_DATA_ROOT)

  # Set CLI testing directories as cache variable so that it can be used by
  # remote module SlicerDMRI included from top-level CMakeLists.txt
  set(BASELINE "${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI"
    CACHE PATH "MRML CLI baseline directory")
  set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input"
    CACHE PATH "MRML CLI testing input data directory")
  set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Core/Testing/TestData"
    CACHE PATH "MRML CLI testing common data directory")
  set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary"
    CACHE PATH "MRML CLI testing temp directory")

  set(MRML_DATA "${Slicer_HOME}/share/MRML/Testing/TestData")
endif()

include(SlicerCheckModuleEnabled)

foreach(module ${cli_modules})
  set(${module}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${module})
  slicer_is_cli_builtin_module_enabled(${module} _build_module)
  if(_build_module)
    add_subdirectory(${module})
  endif()
endforeach()
