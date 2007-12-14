PROJECT (EMSegmentTesting)

INCLUDE_DIRECTORIES ( 
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${VTK_INCLUDE_DIR}
  )

SET( EMSegment_TEST_DIR ${EMSegment_BINARY_DIR} )
SET( EMSegment_TUTORIAL_DIR 
  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest )
SET( EMSegment_TUTORIAL2_DIR 
  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest2 )

# Extra Tests:
# Change these to 1 if you want to run the extra EMSegment tests.  They
# require a testing dataset that is not distributed by default with
# Slicer3.  NB: The extra tests may take a long time and a large
# amount of memory!!!  
SET( EMSEG_RUN_VALGRIND_TESTS    OFF
  CACHE BOOL "Enable EMSegment Valgrind test." FORCE
  )
MARK_AS_ADVANCED(EMSEG_RUN_VALGRIND_TESTS)
SET(EMSEG_RUN_EXTRA_SHORT_TESTS  OFF
  CACHE BOOL "Enable EMSegment extra, short tests." FORCE
  )
MARK_AS_ADVANCED(EMSEG_RUN_EXTRA_SHORT_TESTS)
SET(EMSEG_RUN_EXTRA_LONG_TESTS   OFF
  CACHE BOOL "Enable EMSegment extra, long tests." FORCE
  )
MARK_AS_ADVANCED(EMSEG_RUN_EXTRA_LONG_TESTS)

# Set to 1 to remove temporary test output files
SET(EMSEG_REMOVE_TMP_TEST_DATA   ON
  CACHE BOOL "Remove EMSegment temporary test data." FORCE)
MARK_AS_ADVANCED(EMSEG_REMOVE_TMP_TEST_DATA)

IF( NOT DISABLE_CXX_TESTING )

  ############################################################################
  # add test executables
  ############################################################################ 
  ADD_EXECUTABLE(
    vtkEMSegmentBlackBoxSegmentationTest
    vtkEMSegmentBlackBoxSegmentationTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  TARGET_LINK_LIBRARIES(
    vtkEMSegmentBlackBoxSegmentationTest
    EMSegment
    vtkCommon
    )

  ADD_EXECUTABLE(
    vtkEMSegmentReadWriteMRMLTest
    vtkEMSegmentReadWriteMRMLTest.cxx
    )
  TARGET_LINK_LIBRARIES(
    vtkEMSegmentReadWriteMRMLTest
    EMSegment
    vtkCommon
    )

  ADD_EXECUTABLE(
    vtkEMSegmentMRMLManagerTest
    vtkEMSegmentMRMLManagerTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  TARGET_LINK_LIBRARIES(
    vtkEMSegmentMRMLManagerTest
    EMSegment
    vtkCommon
    )

  ADD_EXECUTABLE(
    vtkEMSegmentPreprocessingManagerTest
    vtkEMSegmentPreprocessingManagerTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  TARGET_LINK_LIBRARIES(
    vtkEMSegmentPreprocessingManagerTest
    EMSegment
    vtkCommon
    )

  ############################################################################
  # The test is a stand-alone executable.  However, the Slicer3
  # launcher is needed to set up shared library paths correctly.
  # We create a prefix for the test executable that accomplishes this
  # wrapping.  
  ############################################################################ 
  SET( SLICER_EXE ${Slicer3_BINARY_DIR}/Slicer3 )
  SET( WRAPPED_EXE_PREFIX --launch ${EXECUTABLE_OUTPUT_PATH})
  IF (WIN32)
    SET( WRAPPED_EXE_PREFIX ${WRAPPED_EXE_PREFIX}/${CMAKE_BUILD_TYPE})
  ENDIF (WIN32)

  ############################################################################
  #
  # command line tests---does the logic work, does it fail elegently
  #
  ############################################################################

  # Does the mrml manager's api function correctly?
  ADD_TEST(
    vtkEMSegmentMRMLManagerTest
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/vtkEMSegmentMRMLManagerTest
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
    "EMSegment Tutorial Template"
    )

  # Does the preprocessing function correctly?
  ADD_TEST(
    vtkEMSegmentPreprocessingManagerTest
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/vtkEMSegmentPreprocessingManagerTest
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
    "EMSegment Tutorial Template"
    )

  # Test that the segmentation results match what the expected
  # results.  This is a legacy test that should not be removed.
  ADD_TEST(
    vtkEMSegmentBlackBoxSegmentationTest_TutorialDataSmallRead
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/vtkEMSegmentBlackBoxSegmentationTest
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    ${EMSegment_TUTORIAL_DIR}
    "EMSegment Tutorial Template"
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.mhd
    )

  # Does help work right?
  ADD_TEST(
    EMSegCL_Help
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --help
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_Help
    PROPERTIES
    PASS_REGULAR_EXPRESSION "USAGE"
    )

  # Does the version argument work right?
  ADD_TEST(
    EMSegCL_Version
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --version
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_Version
    PROPERTIES
    PASS_REGULAR_EXPRESSION "version"
    )

  # Does the segmenter work and give the correct answer using the
  # Tutorial2 default parameters (this includes registration)?
  SET(test_name        "EMSegCL_RunReg")
  SET(test_exe         "${SLICER_EXE}")
  SET(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
  SET(test_cmp_file    "${test_file_prefix}.raw.gz")
  SET(test_std_file    "${EMSegment_TUTORIAL2_DIR}/StandardData/Segmentation_med.raw.gz")
  IF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw.gz)
  ELSE( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_rm_files)
  ENDIF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
  SET(test_args 
    ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName ${EMSegment_TUTORIAL2_DIR}/EMSegmentTestSet2_med.mrml 
    --resultVolumeFileName "${test_file_prefix}.nhdr"
    )
  ADD_TEST(
    ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
    "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
    "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
    "-Drm_files:STRING=${test_rm_files}"
    -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
    )

  # Does the segmenter work and give the correct answer using the
  # default parameters from the small template scene.
  SET(test_name        "EMSegCL_RunDefaultNodes")
  SET(test_exe         "${SLICER_EXE}")
  SET(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
  SET(test_cmp_file    "${test_file_prefix}.raw")
  SET(test_std_file    "${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw")
  IF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw)
  ELSE( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_rm_files)
  ENDIF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
  SET(test_args 
    ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml 
    --resultVolumeFileName "${test_file_prefix}.mhd"
    --disableCompression
    )
  ADD_TEST(
    ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
    "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
    "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
    "-Drm_files:STRING=${test_rm_files}"
    -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
    )

  # Does the segmenter work and give the correct answer when
  # normalization is turned on?
  SET(test_name        "EMSegCL_RunNormalize")
  SET(test_exe         "${SLICER_EXE}")
  SET(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
  SET(test_cmp_file    "${test_file_prefix}.raw")
  SET(test_std_file    "${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small_normalizationOn.raw")
  IF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw)
  ELSE( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_rm_files)
  ENDIF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
  SET(test_args 
    ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml 
    --resultVolumeFileName "${test_file_prefix}.mhd"
    --disableCompression
    )
  ADD_TEST(
    ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
    "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
    "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
    "-Drm_files:STRING=${test_rm_files}"
    -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
    )

  # Does the segmenter work and give the correct answer when
  # everything is specified on the command line?
  SET(test_name        "EMSegCL_RunSetEverything")
  SET(test_exe         "${SLICER_EXE}")
  SET(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
  SET(test_cmp_file    "${test_file_prefix}.raw")
  SET(test_std_file    "${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw")
  IF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw)
  ELSE( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_rm_files)
  ENDIF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
  SET(test_args 
    ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml 
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd
    --atlasVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/atlasBackgroundReg_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/atlasBackgroundReg_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/atlasCSFReg_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/atlasGreymatterReg_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/atlasWhitematterReg_small.mhd
    --parametersMRMLNodeName EMSegment\ Tutorial\ Template
    --resultVolumeFileName "${test_file_prefix}.mhd"
    --disableCompression
    )
  ADD_TEST(
    ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
    "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
    "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
    "-Drm_files:STRING=${test_rm_files}"
    -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
    )

  # Is multithreading disabled when the command line flag is given?
  ADD_TEST(
    EMSegCL_DisableMultithreading
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --disableMultithreading 
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_DisableMultithreading
    PROPERTIES
    PASS_REGULAR_EXPRESSION "Multithreading is disabled"
    )

  # Does it fail elegently when a bogus parameter node is specified?
  ADD_TEST(
    EMSegCL_EFBogusParameterNode
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --parametersMRMLNodeName BogusNodeName  
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusParameterNode
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: no EMSegment parameters found in scene with name ")

  # Does it fail elegently when a bogus mrml scene is specified?
  ADD_TEST(
    EMSegCL_EFBogusMRMLScene
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName /tmp/bogus_file_scene.mrml
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusMRMLScene
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Error: MRML scene file does not exist.")

  # Does it fail elegently when a bogus target images are specified?
  ADD_TEST(
    EMSegCL_EFBogusTargetImages
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/bogus.mhd
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusTargetImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Error: target volume file ")

  # Does it fail elegently when too many target images are specified?
  ADD_TEST(
    EMSegCL_EFTooManyImages
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFTooManyImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: Number of input channels")

  # Does it fail elegently when too few target images are specified?
  ADD_TEST(
    EMSegCL_EFTooFewImages
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFTooFewImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: Number of input channels")

  # Does it fail elegently when a bogus result standard is specified?
  ADD_TEST(
    EMSegCL_EFBogusResultStandardImage
    ${SLICER_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --resultStandardVolumeFileName
    ${EMSegment_TEST_DIR}/bogus_file.mhd
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusResultStandardImage
    PROPERTIES
    PASS_REGULAR_EXPRESSION 
    "Error: result standard volume file does not exist")

  IF ( EMSEG_RUN_EXTRA_LONG_TESTS )
    # run tutorial data
    SET(test_name        "EMSegCL_RunTutorialData")
    SET(test_exe         "${SLICER_EXE}")
    SET(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    SET(test_cmp_file    "${test_file_prefix}.raw")
    SET(test_std_file    "${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult.raw")
    IF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
      SET(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw)
    ELSE( ${EMSEG_REMOVE_TMP_TEST_DATA} )
      SET(test_rm_files)
    ENDIF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_args 
      ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      --verbose 
      --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_large.mrml 
      --resultVolumeFileName "${test_file_prefix}.mhd"
      )
    ADD_TEST(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )
  ENDIF ( EMSEG_RUN_EXTRA_LONG_TESTS )

  ############################################################################
  #
  # MRML---do the logic readers and writers work?
  #
  ############################################################################

  #
  # these tests break every time MRML is modified.  disable them for now.
  #

  IF ( EMSEG_RUN_EXTRA_SHORT_TESTS )
    # Make sure the default parameter set has not changed
    SET(test_name        "EMSegMRML_DiffDefaultNodes")
    SET(test_exe         "${SLICER_EXE}")
    SET(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    SET(test_cmp_file    "${test_file_prefix}.mrml")
    SET(test_std_file    "${EMSegment_SOURCE_DIR}/Testing/TestData/DefaultMRMLNodes.mrml")
    IF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
      SET(test_rm_files    ${test_file_prefix}.mrml)
    ELSE( ${EMSEG_REMOVE_TMP_TEST_DATA} )
      SET(test_rm_files)
    ENDIF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_args 
      ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      --verbose --generateEmptyMRMLSceneAndQuit ${test_cmp_file}
      )
    ADD_TEST(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )
  
    # Test that the mrml readers/writers work.
    SET(test_name        "EMSegMRML_DiffReadWriteNodes")
    SET(test_exe         "${SLICER_EXE}")
    SET(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    SET(test_cmp_file    "${test_file_prefix}.mrml")
    SET(test_std_file    "${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn_noImages.mrml")
    IF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
      SET(test_rm_files    ${test_file_prefix}.mrml)
    ELSE( ${EMSEG_REMOVE_TMP_TEST_DATA} )
      SET(test_rm_files)
    ENDIF( ${EMSEG_REMOVE_TMP_TEST_DATA} )
    SET(test_args 
      ${WRAPPED_EXE_PREFIX}/vtkEMSegmentReadWriteMRMLTest
      ${test_std_file} ${test_cmp_file}
      )
    ADD_TEST(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )
  ENDIF( EMSEG_RUN_EXTRA_SHORT_TESTS )

  ############################################################################
  #
  # Valgrind tests---are there memory leaks?
  #
  ############################################################################

  IF ( EMSEG_RUN_VALGRIND_TESTS )
    ADD_TEST(
      EMSegMRML_ValgrindTest
      valgrind --leak-check=full 
      ${EXECUTABLE_OUTPUT_PATH}/EMSegmentCommandLine
      --verbose 
      --mrmlSceneFileName 
      ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
      --dontWriteResults
      )
  ENDIF ( EMSEG_RUN_VALGRIND_TESTS )

  IF ( EMSEG_RUN_EXTRA_SHORT_TESTS )
    
  ENDIF ( EMSEG_RUN_EXTRA_SHORT_TESTS )

ENDIF( NOT DISABLE_CXX_TESTING )
