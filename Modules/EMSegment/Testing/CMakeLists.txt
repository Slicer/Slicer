project(EMSegmentTesting)

include_directories(
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${VTK_INCLUDE_DIR}
  )

set(EMSegment_TEST_DIR ${EMSegment_BINARY_DIR})
set(EMSegment_TUTORIAL_DIR 
  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest)
set(EMSegment_TUTORIAL2_DIR 
  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest2)

# Extra Tests:
# Change these to 1 if you want to run the extra EMSegment tests.  They
# require a testing dataset that is not distributed by default with
# Slicer3.  NB: The extra tests may take a long time and a large
# amount of memory!!!  
set(EMSEG_RUN_VALGRIND_TESTS    OFF
  CACHE BOOL "Enable EMSegment Valgrind test." FORCE
  )
mark_as_advanced(EMSEG_RUN_VALGRIND_TESTS)
set(EMSEG_RUN_EXTRA_SHORT_TESTS  OFF
  CACHE BOOL "Enable EMSegment extra, short tests." FORCE
  )
mark_as_advanced(EMSEG_RUN_EXTRA_SHORT_TESTS)
set(EMSEG_RUN_EXTRA_LONG_TESTS   OFF
  CACHE BOOL "Enable EMSegment extra, long tests." FORCE
  )
mark_as_advanced(EMSEG_RUN_EXTRA_LONG_TESTS)

mark_as_advanced(EMSEG_RUN_REG_TESTS)
set(EMSEG_RUN_REG_TESTS   OFF
  CACHE BOOL "Enable EMSegment extra, registration tests."
  )

# Set to 1 to remove temporary test output files
set(EMSEG_REMOVE_TMP_TEST_DATA   ON
  CACHE BOOL "Remove EMSegment temporary test data." FORCE)
mark_as_advanced(EMSEG_REMOVE_TMP_TEST_DATA)

if(NOT DISABLE_CXX_TESTING)

  ############################################################################
  # add test executables
  ############################################################################ 
  add_executable(
    vtkEMSegmentBlackBoxSegmentationTest
    vtkEMSegmentBlackBoxSegmentationTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  target_link_libraries(
    vtkEMSegmentBlackBoxSegmentationTest
    EMSegment
    vtkCommon
    )

  add_executable(
    vtkEMSegmentBuildAndRunNewSegmentatationParameters001
    vtkEMSegmentBuildAndRunNewSegmentatationParameters001.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  target_link_libraries(
    vtkEMSegmentBuildAndRunNewSegmentatationParameters001
    EMSegment
    vtkCommon
    )

  add_executable(
    vtkEMSegmentReadWriteMRMLTest
    vtkEMSegmentReadWriteMRMLTest.cxx
    )
  target_link_libraries(
    vtkEMSegmentReadWriteMRMLTest
    EMSegment
    vtkCommon
    )

  add_executable(
    vtkEMSegmentMRMLManagerTest
    vtkEMSegmentMRMLManagerTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  target_link_libraries(
    vtkEMSegmentMRMLManagerTest
    EMSegment
    vtkCommon
    )

  add_executable(
    vtkEMSegmentPreprocessingManagerTest
    vtkEMSegmentPreprocessingManagerTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  target_link_libraries(
    vtkEMSegmentPreprocessingManagerTest
    EMSegment
    vtkCommon
    )

  ############################################################################
  # The test is a stand-alone executable.  However, the Slicer3
  # launcher is needed to set up shared library paths correctly.
  # We create a prefix for the test executable that accomplishes this
  # wrapping.  
  # WRAPPED_TEST_EXE_PREFIX: path to tests that only exist in build tree
  # WRAPPED_EXE_PREFIX: path to command line utilities that install with
  # slicer
  ############################################################################ 
  set(Slicer3_EXE ${Slicer3_BINARY_DIR}/Slicer3)
  set(WRAPPED_TEST_EXE_PREFIX --launch ${EXECUTABLE_OUTPUT_PATH})
  set(WRAPPED_EXE_PREFIX --launch ${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR})
  if(WIN32)
    set(WRAPPED_EXE_PREFIX ${WRAPPED_EXE_PREFIX}/${CMAKE_BUILD_TYPE})
  endif(WIN32)

  ############################################################################
  #
  # command line tests---does the logic work, does it fail elegently
  #
  ############################################################################

  # Does the mrml manager's api function correctly?
  add_test(
    vtkEMSegmentMRMLManagerTest
    ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentMRMLManagerTest
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
    "EMSegment Tutorial Template"
    )

  # Does the preprocessing function correctly?
  add_test(
    vtkEMSegmentPreprocessingManagerTest
    ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentPreprocessingManagerTest
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
    "EMSegment Tutorial Template"
    )

  # Test that the segmentation results match what the expected
  # results.  This is a legacy test that should not be removed.
  add_test(
    vtkEMSegmentBlackBoxSegmentationTest_TutorialDataSmallRead
    ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentBlackBoxSegmentationTest
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    ${EMSegment_TUTORIAL_DIR}
    "EMSegment Tutorial Template"
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.mhd
    )

  # Build parameters from scratch and run the segmentation
  add_test(
    vtkEMSegmentBuildAndRunNewSegmentatationParameters001
    ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentBuildAndRunNewSegmentatationParameters001
    ${EMSegment_SOURCE_DIR}/Testing/TestData/MiscVolumeData/R.mhd
    ${EMSegment_SOURCE_DIR}/Testing/TestData/MiscVolumeData/G.mhd
    ${EMSegment_SOURCE_DIR}/Testing/TestData/MiscVolumeData/B.mhd
    ${EMSegment_TEST_DIR}/RGBSegmentationResult.mhd
    )

  # Does help work right?
  add_test(
    EMSegCL_Help
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --help
    )
  set_tests_properties(
    EMSegCL_Help
    PROPERTIES
    PASS_REGULAR_EXPRESSION "USAGE"
    )

  # Does the version argument work right?
  add_test(
    EMSegCL_Version
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --version
    )
  set_tests_properties(
    EMSegCL_Version
    PROPERTIES
    PASS_REGULAR_EXPRESSION "version"
    )

  if(EMSEG_RUN_REG_TESTS)
    # Does the segmenter work and give the correct answer using the
    # Tutorial2 default parameters (this includes registration)?
    set(test_name        "EMSegCL_RunReg")
    set(test_exe         "${Slicer3_EXE}")
    set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    set(test_cmp_file    "${test_file_prefix}.raw.gz")
    set(test_std_file    "${EMSegment_TUTORIAL2_DIR}/StandardData/Segmentation_med.raw.gz")
    if(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw.gz)
    else(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files)
    endif(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_args 
      ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      --verbose 
      --mrmlSceneFileName ${EMSegment_TUTORIAL2_DIR}/EMSegmentTestSet2_med.mrml 
      --resultVolumeFileName "${test_file_prefix}.nhdr"
      )
    add_test(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )

    # Does the segmenter work and give the correct answer using the
    # Tutorial2 default parameters with no registration?
    set(test_name        "EMSegCL_RunRegNoReg")
    set(test_exe         "${Slicer3_EXE}")
    set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    set(test_cmp_file    "${test_file_prefix}.raw.gz")
    set(test_std_file    "${EMSegment_TUTORIAL2_DIR}/StandardData/Segmentation_NoReg_med.raw.gz")
    if(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw.gz)
    else(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files)
    endif(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_args 
      ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      --verbose 
      --mrmlSceneFileName ${EMSegment_TUTORIAL2_DIR}/EMSegmentTestSet2_NoReg_med.mrml 
      --resultVolumeFileName "${test_file_prefix}.nhdr"
      )
    add_test(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )

    # Does the segmenter work and give the correct answer using the
    # Tutorial2 default parameters with only T2T registration?
    set(test_name        "EMSegCL_RunRegT2T")
    set(test_exe         "${Slicer3_EXE}")
    set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    set(test_cmp_file    "${test_file_prefix}.raw.gz")
    set(test_std_file    "${EMSegment_TUTORIAL2_DIR}/StandardData/Segmentation_T2TReg_med.raw.gz")
    if(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw.gz)
    else(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files)
    endif(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_args 
      ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      --verbose 
      --mrmlSceneFileName ${EMSegment_TUTORIAL2_DIR}/EMSegmentTestSet2_T2TReg_med.mrml 
      --resultVolumeFileName "${test_file_prefix}.nhdr"
      )
    add_test(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )

    # Does the segmenter work and give the correct answer using the
    # Tutorial2 default parameters with only A2T registration?
    set(test_name        "EMSegCL_RunRegA2T")
    set(test_exe         "${Slicer3_EXE}")
    set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    set(test_cmp_file    "${test_file_prefix}.raw.gz")
    set(test_std_file    "${EMSegment_TUTORIAL2_DIR}/StandardData/Segmentation_A2TReg_med.raw.gz")
    if(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw.gz)
    else(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files)
    endif(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_args 
      ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      --verbose 
      --mrmlSceneFileName ${EMSegment_TUTORIAL2_DIR}/EMSegmentTestSet2_A2TReg_med.mrml 
      --resultVolumeFileName "${test_file_prefix}.nhdr"
      )
    add_test(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )
  endif(EMSEG_RUN_REG_TESTS)

  # Does the segmenter work and give the correct answer using the
  # default parameters from the small template scene.
  set(test_name        "EMSegCL_RunDefaultNodes")
  set(test_exe         "${Slicer3_EXE}")
  set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
  set(test_cmp_file    "${test_file_prefix}.raw")
  set(test_std_file    "${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw")
  if(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw)
  else(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_rm_files)
  endif(${EMSEG_REMOVE_TMP_TEST_DATA})
  set(test_args 
    ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml 
    --resultVolumeFileName "${test_file_prefix}.mhd"
    --disableCompression
    )
  add_test(
    ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
    "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
    "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
    "-Drm_files:STRING=${test_rm_files}"
    -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
    )

  # Does the segmenter work and give the correct answer when
  # normalization is turned on?
  set(test_name        "EMSegCL_RunNormalize")
  set(test_exe         "${Slicer3_EXE}")
  set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
  set(test_cmp_file    "${test_file_prefix}.raw")
  set(test_std_file    "${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small_normalizationOn.raw")
  if(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw)
  else(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_rm_files)
  endif(${EMSEG_REMOVE_TMP_TEST_DATA})
  set(test_args 
    ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml 
    --resultVolumeFileName "${test_file_prefix}.mhd"
    --disableCompression
    )
  add_test(
    ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
    "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
    "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
    "-Drm_files:STRING=${test_rm_files}"
    -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
    )

  # Does the segmenter work and give the correct answer when
  # everything is specified on the command line?
  set(test_name        "EMSegCL_RunSetEverything")
  set(test_exe         "${Slicer3_EXE}")
  set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
  set(test_cmp_file    "${test_file_prefix}.raw")
  set(test_std_file    "${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw")
  if(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw)
  else(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_rm_files)
  endif(${EMSEG_REMOVE_TMP_TEST_DATA})
  set(test_args 
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
  add_test(
    ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
    "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
    "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
    "-Drm_files:STRING=${test_rm_files}"
    -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
    )

  # Is multithreading disabled when the command line flag is given?
  add_test(
    EMSegCL_DisableMultithreading
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --disableMultithreading 
    )
  set_tests_properties(
    EMSegCL_DisableMultithreading
    PROPERTIES
    PASS_REGULAR_EXPRESSION "Multithreading is disabled"
    )

  # Does it fail elegently when a bogus parameter node is specified?
  add_test(
    EMSegCL_EFBogusParameterNode
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --parametersMRMLNodeName BogusNodeName  
    )
  set_tests_properties(
    EMSegCL_EFBogusParameterNode
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: no EMSegment parameters found in scene with name ")

  # Does it fail elegently when a bogus mrml scene is specified?
  add_test(
    EMSegCL_EFBogusMRMLScene
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName /tmp/bogus_file_scene.mrml
    )
  set_tests_properties(
    EMSegCL_EFBogusMRMLScene
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Error: MRML scene file does not exist.")

  # Does it fail elegently when a bogus target images are specified?
  add_test(
    EMSegCL_EFBogusTargetImages
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/bogus.mhd
    )
  set_tests_properties(
    EMSegCL_EFBogusTargetImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Error: target volume file ")

  # Does it fail elegently when too many target images are specified?
  add_test(
    EMSegCL_EFTooManyImages
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd
    )
  set_tests_properties(
    EMSegCL_EFTooManyImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: Number of input channels")

  # Does it fail elegently when too few target images are specified?
  add_test(
    EMSegCL_EFTooFewImages
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd
    )
  set_tests_properties(
    EMSegCL_EFTooFewImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: Number of input channels")

  # Does it fail elegently when a bogus result standard is specified?
  add_test(
    EMSegCL_EFBogusResultStandardImage
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --resultStandardVolumeFileName
    ${EMSegment_TEST_DIR}/bogus_file.mhd
    )
  set_tests_properties(
    EMSegCL_EFBogusResultStandardImage
    PROPERTIES
    PASS_REGULAR_EXPRESSION 
    "Error: result standard volume file does not exist")

  if(EMSEG_RUN_EXTRA_LONG_TESTS)
    # run tutorial data
    set(test_name        "EMSegCL_RunTutorialData")
    set(test_exe         "${Slicer3_EXE}")
    set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    set(test_cmp_file    "${test_file_prefix}.raw")
    set(test_std_file    "${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult.raw")
    if(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files    ${test_file_prefix}.mhd ${test_file_prefix}.raw)
    else(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files)
    endif(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_args 
      ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      --verbose 
      --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_large.mrml 
      --resultVolumeFileName "${test_file_prefix}.mhd"
      )
    add_test(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )
  endif(EMSEG_RUN_EXTRA_LONG_TESTS)

  ############################################################################
  #
  # MRML---do the logic readers and writers work?
  #
  ############################################################################

  #
  # these tests break every time MRML is modified.  disable them for now.
  #

  if(EMSEG_RUN_EXTRA_SHORT_TESTS)
    # Make sure the default parameter set has not changed
    set(test_name        "EMSegMRML_DiffDefaultNodes")
    set(test_exe         "${Slicer3_EXE}")
    set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    set(test_cmp_file    "${test_file_prefix}.mrml")
    set(test_std_file    "${EMSegment_SOURCE_DIR}/Testing/TestData/DefaultMRMLNodes.mrml")
    if(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files    ${test_file_prefix}.mrml)
    else(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files)
    endif(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_args 
      ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      --verbose --generateEmptyMRMLSceneAndQuit ${test_cmp_file}
      )
    add_test(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )
    
    # Test that the mrml readers/writers work.
    set(test_name        "EMSegMRML_DiffReadWriteNodes")
    set(test_exe         "${Slicer3_EXE}")
    set(test_file_prefix "${EMSegment_TEST_DIR}/${test_name}_tmp")
    set(test_cmp_file    "${test_file_prefix}.mrml")
    set(test_std_file    "${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn_noImages.mrml")
    if(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files    ${test_file_prefix}.mrml)
    else(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files)
    endif(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_args 
      ${WRAPPED_EXE_PREFIX}/vtkEMSegmentReadWriteMRMLTest
      ${test_std_file} ${test_cmp_file}
      )
    add_test(
      ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
      "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
      "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
      "-Drm_files:STRING=${test_rm_files}"
      -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
      )
  endif(EMSEG_RUN_EXTRA_SHORT_TESTS)

  ############################################################################
  #
  # Valgrind tests---are there memory leaks?
  #
  ############################################################################

  if(EMSEG_RUN_VALGRIND_TESTS)
    add_test(
      EMSegMRML_ValgrindTest
      valgrind --leak-check=full 
      ${EXECUTABLE_OUTPUT_PATH}/EMSegmentCommandLine
      --verbose 
      --mrmlSceneFileName 
      ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
      --dontWriteResults
      )
  endif(EMSEG_RUN_VALGRIND_TESTS)

  if(EMSEG_RUN_EXTRA_SHORT_TESTS)
    
  endif(EMSEG_RUN_EXTRA_SHORT_TESTS)

endif(NOT DISABLE_CXX_TESTING)
