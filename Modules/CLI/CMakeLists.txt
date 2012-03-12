

#-----------------------------------------------------------------------------
# WARNING: CLI modules should be topologically ordered.
#-----------------------------------------------------------------------------
set(cli_modules
  ACPCTransform
  AddScalarVolumes
  CastScalarVolume
  CheckerBoardFilter
  CreateDICOMSeries
  CurvatureAnisotropicDiffusion
  DiffusionTensorScalarMeasurements
  DiffusionWeightedVolumeMasking
  DWIJointRicianLMMSEFilter
  DWIRicianLMMSEFilter
  DWIToDTIEstimation
  DWIUnbiasedNonLocalMeansFilter
  DTIImportExport
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
  Java
  LabelMapSmoothing
  MaskScalarVolume
  MedianImageFilter
  MergeModels
  MeshContourSegmentation
  ModelMaker
  ModelToLabelMap
  MultiplyScalarVolumes
  N4ITKBiasFieldCorrection
  OrientScalarVolume
  ProbeVolumeWithModel
  ResampleDTIVolume
  ResampleScalarVectorDWIVolume # Depends on DiffusionApplications
  RobustStatisticsSegmenter
  SimpleRegionGrowingSegmentation
  SubtractScalarVolumes
  ThresholdScalarVolume
  TractographyLabelMapSeeding
  VotingBinaryHoleFillingImageFilter
  )

if(Slicer_BUILD_LEGACY_CLI)
  set(legacy_cli_modules
    AffineRegistration
    BSplineDeformableRegistration
    BSplineToDeformationField
    DiffusionTensorTest
    ExpertAutomatedRegistration
    FiducialRegistration
    LinearRegistration
    MultiResolutionAffineRegistration
    OtsuThresholdImageFilter
    OtsuThresholdSegmentation
    ResampleScalarVolume
    RigidRegistration
    TestGridTransformRegistration
    )
  list(APPEND cli_modules ${legacy_cli_modules})
  if("${ITK_VERSION_MAJOR}" LESS 4)
    ## The time necessary to rewrite this legacy application to work in ITKv4 is trivial, to make it work in
    ## both ITKv4 and ITKv3 is painful.
    list(APPEND cli_modules MRIBiasFieldCorrection)
  endif()
endif()

if (Slicer_BUILD_EMSegment)
  ## EMSegment requires some LEGACY_CLI to work. Add them if not already added
  if (NOT Slicer_BUILD_LEGACY_CLI)
    list(APPEND cli_modules BSplineToDeformationField)
  endif()
endif()

if("${ITK_VERSION_MAJOR}" LESS 4)
  list(APPEND cli_modules DicomToNrrdConverter)
  list(APPEND cli_modules PETStandardUptakeValueComputation)
endif()

if(Slicer_USE_MIDAS)
  list(APPEND cli_modules MIDASApplications)
endif()

if(Slicer_USE_BatchMake)
  list(APPEND cli_modules BatchMakeApplications)
endif()

if(BUILD_TESTING)
  # TODO Since the following snippet have been moved from CLI/Testing/CMakeLists.txt into
  # CLI/CMakeLists.txt, the variable SLICER_BRAINWEB_DATA_ROOT won't be set properly.
  # Configure the default SLICER_BRAINWEB_DATA_ROOT for the location of BrainWeb Data.
  # When this data is available, additional 3D tests are enabled.
  find_path(SLICER_BRAINWEB_DATA_ROOT brainweb165a10f17.mha
    DOC "Directory with data taken from http://public.kitware.com/pub/itk/Data/BrainWeb/")
  mark_as_advanced(SLICER_BRAINWEB_DATA_ROOT)

  set(Launcher_Command ${Slicer_LAUNCH_COMMAND})
  if(USE_VALGRIND)
    set(Launcher_Command "${Slicer_BINARY_DIR}/Scripts/runValgrindForOneModule.sh")
  endif()

  set(BASELINE "${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI")
  set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
  set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Core/Testing/TestData")
  set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

  set(MRML_DATA "${Slicer_HOME}/share/MRML/Testing/TestData")
  if(ITK_VERSION_MAJOR GREATER 3)
    include_directories(${Slicer_SOURCE_DIR}/Modules/CLI)
  endif()
endif()

foreach(cli_module ${cli_modules})
  add_subdirectory(${cli_module})
endforeach()

if(Slicer_BUILD_BRAINSTOOLS)
  # NOTE: BRAINSTools source code is checkout using "External_BRAINSTools.cmake".
  set(BRAINSTools_CLI_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_INSTALL_LIBRARY_DESTINATION ${Slicer_INSTALL_CLIMODULES_LIB_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_INSTALL_ARCHIVE_DESTINATION ${Slicer_INSTALL_CLIMODULES_LIB_DIR} CACHE PATH "" FORCE)
  set(BRAINSTools_CLI_INSTALL_RUNTIME_DESTINATION ${Slicer_INSTALL_CLIMODULES_BIN_DIR} CACHE PATH "" FORCE)

  if(BUILD_TESTING)
    set(LAUNCH_EXE ${Slicer_LAUNCH_COMMAND})
  endif()

  set(USE_BRAINSFit                       ON CACHE BOOL "Build BRAINSFit"       )
  set(USE_BRAINSROIAuto                   ON CACHE BOOL "Build BRAINSROIAuto"   )
  set(USE_BRAINSResample                  ON CACHE BOOL "Build BRAINSResample"  )
  set(USE_BRAINSDemonWarp                 ON CACHE BOOL "Build BRAINSDemonWarp ")
  ## BRAINSTools comes with some extra tool
  ## options that should not be compiled by default.
  set(USE_GTRACT                         OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSABC                      OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSConstellationDetector    OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSMush                     OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSInitializedControlPoints OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSMultiModeSegment         OFF CACHE BOOL "" FORCE)
  set(USE_BRAINSCut                      OFF CACHE BOOL "" FORCE)
  set(USE_ImageCalculator                OFF CACHE BOOL "" FORCE)

  set(ITK_VERSION_MAJOR ${ITK_VERSION_MAJOR} CACHE STRING "")
  set(BRAINSTools_SUPERBUILD             OFF CACHE BOOL "" FORCE)
  add_subdirectory(
    ${BRAINSTools_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/BRAINSTools
    )
endif()

if(Slicer_BUILD_SkullStripper)
  # NOTE: SkullStripper source code is checkout using "External_SkullStripper.cmake".
  add_subdirectory(
    ${SkullStripper_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/SkullStripper
    )
endif()

