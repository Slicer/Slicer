#-----------------------------------------------------------------------------
# WARNING: CLI modules should be topologically ordered.
#-----------------------------------------------------------------------------
set(cli_modules
  Add
  AffineRegistration
  BSplineDeformableRegistration
  BSplineToDeformationField
  Cast
  CheckerBoard
  ComputeSUVBodyWeight
  ConfidenceConnected
  CurvatureAnisotropicDiffusion
  DiffusionApplications
  DiffusionTensorEstimation
  DiffusionTensorMathematics
  DiffusionTensorTest
  DiffusionWeightedMasking
  DiffusionWeightedTest
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
  ImageReadDicomWrite
  Java
  LabelMapSmoothing
  LinearRegistration
  Mask
  MedianImageFilter
  MergeModels
  ModelMaker
  MRIBiasFieldCorrection
  MultipleModelsExample
  Multiply
  N4ITKBiasFieldCorrection
  OrientImage
  OtsuThresholdImageFilter
  OtsuThresholdSegmentation
  PolyDataToLabelmap
  ProbeVolumeWithModel
  Realign
  RegisterImagesModule
  ResampleVolume2
  ResampleVolume
  RigidRegistration
  RobustStatisticsSegmenter
  Seeding
  SparseFieldLevelSetContour
  Subtract
  TestGridTransformRegistration
  Threshold
  VotingBinaryHoleFillingImageFilter
  ZeroCrossingBasedEdgeDetectionImageFilter
  )

if("${ITK_VERSION_MAJOR}" LESS 4)
  list(APPEND cli_modules DicomToNrrdConverter)
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
  set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData")
  set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

  set(MRML_DATA "${Slicer_HOME}/share/MRML/Testing/TestData")
  if(ITK_VERSION_MAJOR GREATER 3)
    include_directories(${Slicer_SOURCE_DIR}/Applications/CLI)
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

  set(ITK_VERSION_MAJOR ${ITK_VERSION_MAJOR} CACHE STRING "")
  set(BRAINSTools_SUPERBUILD OFF CACHE BOOL "" FORCE)
  add_subdirectory(
    ${BRAINSTools_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/BRAINSTools
    )
endif()
