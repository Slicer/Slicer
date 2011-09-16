project(EMSegmentTesting)

include_directories(
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${VTK_INCLUDE_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${vtkSlicerAtlasCreatorModuleLogic_SOURCE_DIR}
  ${vtkSlicerAtlasCreatorModuleLogic_BINARY_DIR}    
  ${EMSegment_BINARY_DIR}
  )

set(EMSegment_TEST_DIR ${EMSegment_BINARY_DIR})
set(EMSegment_TUTORIAL_DIR
  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest)
set(EMSegment_TUTORIAL2_DIR
  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest2)
set(EMSegment_TESTDATA_DIR
  ${EMSegment_SOURCE_DIR}/Testing/TestData)

# target link libraries depend on the Slicer version

if(NOT EM_Slicer4_FOUND)

# Slicer3

set(TEST_LINK_LIBRARIES
    EMSegment
    vtkCommon
    )
    
set(EMS_SLICER_LAUNCH_ARGUMENTS --no-error-popup)
    
else(NOT EM_Slicer4_FOUND)

# Slicer4

set(TEST_LINK_LIBRARIES
    vtkCommon
    vtkSlicerEMSegmentModuleLogic
    )

set(EMS_SLICER_LAUNCH_ARGUMENTS "")

endif(NOT EM_Slicer4_FOUND)


#### MIDAS
# http://www.kitware.com/midaswiki/index.php/MIDAS%2BCTest
include(MIDAS)
if (NOT MIDAS_REST_URL)
  set(MIDAS_REST_URL "http://midas.kitware.com/api/rest" CACHE STRING "The MIDAS server where testing data resides")
endif (NOT MIDAS_REST_URL)
set(MIDAS_KEY_DIR "${EMSegment_SOURCE_DIR}/Testing/TestData/MIDAS_Keys")
#### MIDAS


# Extra Tests:
# Change these to 1 if you want to run the extra EMSegment tests.  They
# require a testing dataset that is not distributed by default with
# Slicer3.  NB: The extra tests may take a long time and a large
# amount of memory!!!


set(EMSEG_RUN_OLD_TESTS   OFF
  CACHE BOOL "Enable old EMSegment tests." FORCE
  )
mark_as_advanced(EMSEG_RUN_OLD_TEST)

set(EMSEG_RUN_COMPARE_TESTS   OFF
  CACHE BOOL "Enable EMSegment tests to compare segmentation results." FORCE
  )
mark_as_advanced(EMSEG_RUN_COMPARE_TESTS)

set(EMSEG_RUN_LOCAL_TESTS   OFF
  CACHE BOOL "Enable local EMSegment tests." FORCE
  )
mark_as_advanced(EMSEG_RUN_LOCAL_TESTS)

set(EMSEG_RUN_VALGRIND_TESTS    OFF
  CACHE BOOL "Enable EMSegment Valgrind test." FORCE
  )
mark_as_advanced(EMSEG_RUN_VALGRIND_TESTS)

set(EMSEG_RUN_EXTRA_SHORT_TESTS  ON
  CACHE BOOL "Enable EMSegment extra, short tests." FORCE
  )
mark_as_advanced(EMSEG_RUN_EXTRA_SHORT_TESTS)

set(EMSEG_RUN_EXTRA_LONG_TESTS   OFF
  CACHE BOOL "Enable EMSegment extra, long tests." FORCE
  )
mark_as_advanced(EMSEG_RUN_EXTRA_LONG_TESTS)

set(EMSEG_RUN_REG_TESTS   OFF
  CACHE BOOL "Enable EMSegment extra, registration tests." FORCE
  )
mark_as_advanced(EMSEG_RUN_REG_TESTS)


# Set to 1 to remove temporary test output files
set(EMSEG_REMOVE_TMP_TEST_DATA   "1"
  CACHE BOOL "Remove EMSegment temporary test data." FORCE)
mark_as_advanced(EMSEG_REMOVE_TMP_TEST_DATA)

if(NOT DISABLE_CXX_TESTING)

  ############################################################################
  # add test executables
  ############################################################################
  add_executable(
    vtkSlicerCommonInterfaceTest1
    vtkSlicerCommonInterfaceTest1.cxx
    )
  target_link_libraries(
    vtkSlicerCommonInterfaceTest1
    ${TEST_LINK_LIBRARIES}
    )
  
  add_executable(
    vtkEMSegmentRemoteIOTest
    vtkEMSegmentRemoteIOTest.cxx
    )
  target_link_libraries(
    vtkEMSegmentRemoteIOTest
    RemoteIO
    ${TEST_LINK_LIBRARIES}
    )

  add_executable(
    vtkEMSegmentBlackBoxSegmentationTest
    vtkEMSegmentBlackBoxSegmentationTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  target_link_libraries(
    vtkEMSegmentBlackBoxSegmentationTest
    ${TEST_LINK_LIBRARIES}
    )

  add_executable(
    vtkEMSegmentCompareTest
    vtkEMSegmentCompareTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  target_link_libraries(
    vtkEMSegmentCompareTest
    ${TEST_LINK_LIBRARIES}
    )

  add_executable(
    vtkEMSegmentReadWriteMRMLTest
    vtkEMSegmentReadWriteMRMLTest.cxx
    )
  target_link_libraries(
    vtkEMSegmentReadWriteMRMLTest
    ${TEST_LINK_LIBRARIES}
    )

  add_executable(
    vtkEMSegmentMRMLManagerTest
    vtkEMSegmentMRMLManagerTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  target_link_libraries(
    vtkEMSegmentMRMLManagerTest
    ${TEST_LINK_LIBRARIES}
    )

  add_executable(
    vtkEMSegmentPreprocessingManagerTest
    vtkEMSegmentPreprocessingManagerTest.cxx
    vtkEMSegmentTestUtilities.cxx
    )
  target_link_libraries(
    vtkEMSegmentPreprocessingManagerTest
    ${TEST_LINK_LIBRARIES}
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
  if(NOT EM_Slicer4_FOUND)

    # Slicer3
  
    set(Slicer3_EXE ${Slicer3_BINARY_DIR}/Slicer3)
    set(WRAPPED_TEST_EXE_PREFIX --launch ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(WRAPPED_EXE_PREFIX --launch ${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR})
    if(WIN32)
      set(WRAPPED_EXE_PREFIX ${WRAPPED_EXE_PREFIX}/${CMAKE_BUILD_TYPE})
      set(WRAPPED_TEST_EXE_PREFIX ${WRAPPED_TEST_EXE_PREFIX}/${CMAKE_BUILD_TYPE})
    endif(WIN32)
    
  else(NOT EM_Slicer4_FOUND)
  
    # Slicer4
    set(Slicer3_BINARY_DIR ${Slicer_BINARY_DIR})
    set(Slicer3_EXE ${Slicer_BINARY_DIR}/Slicer --launcher-no-splash)
    set(WRAPPED_TEST_EXE_PREFIX --launch ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(WRAPPED_EXE_PREFIX --launch ${Slicer_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR})
    if(WIN32)
      set(WRAPPED_EXE_PREFIX ${WRAPPED_EXE_PREFIX}/${CMAKE_BUILD_TYPE})
      set(WRAPPED_TEST_EXE_PREFIX ${WRAPPED_TEST_EXE_PREFIX}/${CMAKE_BUILD_TYPE})
    endif(WIN32)
  
  endif(NOT EM_Slicer4_FOUND)

  ############################################################################
  #
  # command line tests---does the logic work, does it fail elegently
  #
  ############################################################################

  # Does the mrml manager's api function correctly?
  add_test( vtkEMSegmentMRMLManagerTest
    ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentMRMLManagerTest
    ${EMSegment_TUTORIAL_DIR}/Template_small_normalizationOn.mrml
    "EMSegment Tutorial Template"
    )

  # Does the preprocessing function correctly?
  #add_test( vtkEMSegmentPreprocessingManagerTest
  #  ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentPreprocessingManagerTest
  #  ${EMSegment_TUTORIAL_DIR}/Template_small_normalizationOn.mrml
  #  "EMSegment Tutorial Template"
  #  )

  # Test that the segmentation results match what the expected
  # results.  This is a legacy test that should not be removed.
  add_test( vtkEMSegmentBlackBoxSegmentationTest_TutorialDataSmallRead
    ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentBlackBoxSegmentationTest
    ${EMSegment_TUTORIAL_DIR}/Template_small.mrml
    ${EMSegment_TUTORIAL_DIR}
    "EMSegment Tutorial Template"
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.mhd
    )
    
  add_test( vtkSlicerCommonInterfaceTest1
    ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkSlicerCommonInterfaceTest1
    )
        

  # Build parameters from scratch and run the segmentation
  #add_test( vtkEMSegmentBuildAndRunNewSegmentationParameters001
  #  ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentBuildAndRunNewSegmentationParameters001
  #  ${EMSegment_SOURCE_DIR}/Testing/TestData/MiscVolumeData/R.mhd
  #  ${EMSegment_SOURCE_DIR}/Testing/TestData/MiscVolumeData/G.mhd
  #  ${EMSegment_SOURCE_DIR}/Testing/TestData/MiscVolumeData/B.mhd
  #  ${EMSegment_TEST_DIR}/RGBSegmentationResult.mhd
  #  )

  # Does help work right?
  add_test( EMSegCL_Help
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --help
    )
  set_tests_properties(
    EMSegCL_Help
    PROPERTIES
    PASS_REGULAR_EXPRESSION "USAGE"
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
    add_test( ${test_name}
      ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
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
    add_test( ${test_name}
      ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
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
    add_test( ${test_name}
      ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
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
    add_test( ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
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
    --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/Template_small.mrml 
    --resultVolumeFileName "${test_file_prefix}.mhd"
    --disableCompression
    )
  add_test( ${test_name}
    ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
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
    --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/Template_small_normalizationOn.mrml 
    --resultVolumeFileName "${test_file_prefix}.mhd"
    --disableCompression
    )
  #add_test( ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
  #  "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
  #  "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
  #  "-Drm_files:STRING=${test_rm_files}"
  #  -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
  #  )

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
    --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/Template_small.mrml 
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd
    --parametersMRMLNodeName EMSegment\ Tutorial\ Template
    --resultVolumeFileName "${test_file_prefix}.mhd"
    --disableCompression
    )
  add_test( ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
    "-Dtest_exe:STRING=${test_exe}"      "-Dtest_exe_args:STRING=${test_args}"
    "-Dfile_cmp:STRING=${test_cmp_file}" "-Dfile_std:STRING=${test_std_file}"
    "-Drm_files:STRING=${test_rm_files}"
    -P "${EMSegment_SOURCE_DIR}/Testing/RunDiffRemoveTest.cmake"
    )

  # Is multithreading disabled when the command line flag is given?
  add_test( EMSegCL_DisableMultithreading
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/Template_small.mrml
    --disableMultithreading -1
    )
  set_tests_properties(
    EMSegCL_DisableMultithreading
    PROPERTIES
    PASS_REGULAR_EXPRESSION "Multithreading is disabled"
    )

  # Does it fail elegently when a bogus parameter node is specified?
  add_test( EMSegCL_EFBogusParameterNode
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/Template_small.mrml
    --parametersMRMLNodeName BogusNodeName  
    )
  set_tests_properties(
    EMSegCL_EFBogusParameterNode
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: no EMSegment parameters found in scene with name ")

  # Does it fail elegently when a bogus mrml scene is specified?
  add_test( EMSegCL_EFBogusMRMLScene
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --dontWriteResults --mrmlSceneFileName ${Slicer3_BINARY_DIR}/Testing/Temporary/bogus_file_scene.mrml
    )
  set_tests_properties(
    EMSegCL_EFBogusMRMLScene
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Error: MRML scene file does not exist.")

  # Does it fail elegently when a bogus target images are specified?
  add_test( EMSegCL_EFBogusTargetImages
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/Template_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/bogus.mhd
    )
  set_tests_properties(
    EMSegCL_EFBogusTargetImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Error: target volume file ")

  # Does it fail elegently when too many target images are specified?
  add_test( EMSegCL_EFTooManyImages
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/Template_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd
    )
  set_tests_properties(
    EMSegCL_EFTooManyImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: Number of input channels")

  # Does it fail elegently when too few target images are specified?
  add_test( EMSegCL_EFTooFewImages
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --dontWriteResults --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/Template_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd
    )
  set_tests_properties(
    EMSegCL_EFTooFewImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: Number of input channels")

  # Does the option --intermediateResultsDirectory work?
  add_test( EMSegCL_IntermediateResults
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --mrmlSceneFileName
    ${EMSegment_TUTORIAL_DIR}/Template_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd
    --resultVolumeFileName
    ${Slicer3_BINARY_DIR}/Testing/Temporary/EMSegCL_IntermediateResults_FinalResult.nrrd
    --intermediateResultsDirectory
    ${Slicer3_BINARY_DIR}/Testing/Temporary/EMSegCL_IntermediateResults_FinalResultsFolder
    )
  set_tests_properties(
    EMSegCL_IntermediateResults
    PROPERTIES
    FAIL_REGULAR_EXPRESSION
    "ERROR;Error writing intermediate results"
    )


  # Does it fail elegently when a bogus result standard is specified?
  add_test( EMSegCL_EFBogusResultStandardImage
    ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --dontWriteResults --mrmlSceneFileName
    ${EMSegment_TUTORIAL_DIR}/Template_small.mrml
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
      --mrmlSceneFileName ${EMSegment_TUTORIAL_DIR}/Template_large.mrml
      --resultVolumeFileName "${test_file_prefix}.mhd"
      )
    add_test( ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
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
    add_test( ${test_name} ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
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

if(EM_Slicer4_FOUND)
    set(test_std_file    "${EMSegment_TUTORIAL_DIR}/Template_Slicer4_small_normalizationOn_noImages.mrml")
else(EM_Slicer4_FOUND)
    set(test_std_file    "${EMSegment_TUTORIAL_DIR}/Template_small_normalizationOn_noImages.mrml")
endif(EM_Slicer4_FOUND)

    if(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files    ${test_file_prefix}.mrml)
    else(${EMSEG_REMOVE_TMP_TEST_DATA})
      set(test_rm_files)
    endif(${EMSEG_REMOVE_TMP_TEST_DATA})
    set(test_args
      ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentReadWriteMRMLTest
      ${test_std_file} ${test_cmp_file}
      )
    add_test( ${test_name}
      ${CMAKE_COMMAND} -V -VV "-Dtest_name:STRING=${test_name}"
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
    add_test( EMSegMRML_ValgrindTest
      valgrind --leak-check=full
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EMSegmentCommandLine
      --verbose
      --mrmlSceneFileName
      ${EMSegment_TUTORIAL_DIR}/Template_small_normalizationOn.mrml
      --dontWriteResults
      )
  endif(EMSEG_RUN_VALGRIND_TESTS)


  if (EMSEG_RUN_LOCAL_TESTS)
    set (registrationpackages CMTK BRAINS PLASTIMATCH DEMONS DRAMMS ANTS)
    set (scalartypes float char double int short uchar uint ushort)
    set (sizes small very_small)
  else (EMSEG_RUN_LOCAL_TESTS)
    set (registrationpackages BRAINS)
    set (scalartypes float uint)
    set (sizes small)
  endif (EMSEG_RUN_LOCAL_TESTS)



  #==== Scalartype tests ============================
  set (size very_small)
  foreach (registrationpackage ${registrationpackages})

    foreach (scalartype ${scalartypes})

      # Task test - scalartypes - registrationpackages
      midas_add_test( EMSegCL_Task_${registrationpackage}_MRIHumanBrain_scalartype_${scalartype}_${size}
        ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
        ${EMS_SLICER_LAUNCH_ARGUMENTS}
        --verbose --mrmlSceneFileName
        ${EMSegment_SOURCE_DIR}/Tasks/MRI-Human-Brain.mrml
        --targetVolumeFileNames
        MIDAS{MRI-Human-Brain_T1-Atlas-Template-${scalartype}_${size}.nrrd.md5}
        --resultVolumeFileName
        ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_MRI-Human-Brain_T1-Atlas-Template-${scalartype}_${size}.nrrd
        --taskPreProcessingSetting
        ":C1:C0"
        --registrationPackage
        ${registrationpackage}
        --registrationAffineType
        -1
        --registrationDeformableType
        -1
        --atlasVolumeFileNames
        ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_skulneck.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_air.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_csf.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_greymatter.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_whitematter.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_t1.nrrd
        )
      set_tests_properties(
        EMSegCL_Task_${registrationpackage}_MRIHumanBrain_scalartype_${scalartype}_${size}
        PROPERTIES
        FAIL_REGULAR_EXPRESSION
        "ERROR"
        )

    endforeach (scalartype ${scalartypes})

  endforeach (registrationpackage ${registrationpackages})



  foreach (size ${sizes})

    foreach (registrationpackage ${registrationpackages})

    # Task MRI-Human-Brain
    add_test(
      EMSegCL_Task_MRIHumanBrain_${registrationpackage}_${size}
      ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      ${EMS_SLICER_LAUNCH_ARGUMENTS}
      --verbose --mrmlSceneFileName
      ${EMSegment_SOURCE_DIR}/Tasks/MRI-Human-Brain.mrml
      --targetVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_MRIHumanBrain_T1_aligned.nrrd
      --resultVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrain_T1_aligned.nrrd
      --taskPreProcessingSetting
      ":C1:C0"
      --registrationPackage
      ${registrationpackage}
      --registrationAffineType
      -1
      --registrationDeformableType
      -1
      --atlasVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_skulneck.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_air.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_csf.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_greymatter.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_whitematter.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_t1.nrrd
      )
    set_tests_properties(
      EMSegCL_Task_MRIHumanBrain_${registrationpackage}_${size}
      PROPERTIES
      FAIL_REGULAR_EXPRESSION
      "ERROR"
      )
    if(EMSEG_RUN_COMPARE_TESTS)
      midas_add_test(
        EMSegCL_Task_MRIHumanBrain_${registrationpackage}_${size}_CompareTest
        ${Slicer3_EXE} --launch vtkEMSegmentCompareTest ${EMS_SLICER_LAUNCH_ARGUMENTS}
        ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrain_T1_aligned.nrrd
                          MIDAS{RESULT_${registrationpackage}_${size}_MRIHumanBrain_T1_aligned.nrrd.md5}
        )
    endif(EMSEG_RUN_COMPARE_TESTS)


    # Task Human-Eye
    add_test(
      EMSegCL_Task_HumanEye_${registrationpackage}_${size}
      ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      ${EMS_SLICER_LAUNCH_ARGUMENTS}
      --verbose --mrmlSceneFileName
      ${EMSegment_SOURCE_DIR}/Tasks/Human-Eye.mrml
      --targetVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_MRIHumanBrain_T1_aligned.nrrd
      --resultVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanEye.nrrd
      --taskPreProcessingSetting
      ":C1:C0"
      --registrationPackage
      ${registrationpackage}
      --registrationAffineType
      -1
      --registrationDeformableType
      -1
      )
    set_tests_properties(
      EMSegCL_Task_HumanEye_${registrationpackage}_${size}
      PROPERTIES
      FAIL_REGULAR_EXPRESSION
      "ERROR"
      )
#    if(EMSEG_RUN_COMPARE_TESTS)
#      midas_add_test(
#        EMSegCL_Task_MRIHumanBrain_${registrationpackage}_${size}_CompareTest
#        ${Slicer3_EXE} --launch vtkEMSegmentCompareTest ${EMS_SLICER_LAUNCH_ARGUMENTS}
#        ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrain_T1_aligned.nrrd
#                          MIDAS{RESULT_${registrationpackage}_${size}_MRIHumanBrain_T1_aligned.nrrd.md5}
#        )
#    endif(EMSEG_RUN_COMPARE_TESTS)


    #TODO: all test *nrrd files are taken from other tasks, generate new ones
    # this task is using GenericTask.tcl, otherwise the logmean value will be overwritten and the bug is not visible
    # Task Human-Eye_LogMean
    # deactivate this test, the test should fail
    if (EMSEG_RUN_LOCAL_TESTS)
    add_test(
      EMSegCL_Task_HumanEye_LogMean_${registrationpackage}_${size}
      ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      ${EMS_SLICER_LAUNCH_ARGUMENTS}
      --verbose --mrmlSceneFileName
      ${EMSegment_SOURCE_DIR}/Testing/TestData/Human-Eye_LogMeanError.mrml
      --targetVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_MRIHumanBrain_T1_aligned.nrrd
      --resultVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanEye.nrrd
      --taskPreProcessingSetting
      ":C0:C0"
      --registrationPackage
      ${registrationpackage}
      --registrationAffineType
      -1
      --registrationDeformableType
      -1
      )
    set_tests_properties(
      EMSegCL_Task_HumanEye_LogMean_${registrationpackage}_${size}
      PROPERTIES
      FAIL_REGULAR_EXPRESSION
      "ERROR"
      )
    endif (EMSEG_RUN_LOCAL_TESTS)

#    if(EMSEG_RUN_COMPARE_TESTS)
#      midas_add_test(
#        EMSegCL_Task_MRIHumanBrain_${registrationpackage}_${size}_CompareTest
#        ${Slicer3_EXE} --launch vtkEMSegmentCompareTest ${EMS_SLICER_LAUNCH_ARGUMENTS}
#        ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrain_T1_aligned.nrrd
#                          MIDAS{RESULT_${registrationpackage}_${size}_MRIHumanBrain_T1_aligned.nrrd.md5}
#        )
#    endif(EMSEG_RUN_COMPARE_TESTS)



    # Task MRI-Human-Brain-Exp
    add_test(
      EMSegCL_Task_MRIHumanBrainExp_${registrationpackage}_${size}
      ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      ${EMS_SLICER_LAUNCH_ARGUMENTS}
      --verbose --mrmlSceneFileName
      ${EMSegment_SOURCE_DIR}/Tasks/MRI-Human-Brain-Exp.mrml
      --targetVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_MRIHumanBrain_T1_aligned.nrrd
      --resultVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrainExp_T1_aligned.nrrd
      --taskPreProcessingSetting
      ":C1:C0:C1"
      --registrationPackage
      ${registrationpackage}
      --atlasVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_skulneck.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_air.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_csf.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_greymatter.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_whitematter.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_atlas_t1_stripped.nrrd
      )
    set_tests_properties(
      EMSegCL_Task_MRIHumanBrainExp_${registrationpackage}_${size}
      PROPERTIES
      FAIL_REGULAR_EXPRESSION
      "ERROR"
      )
    if(EMSEG_RUN_COMPARE_TESTS)
      midas_add_test(
        EMSegCL_Task_MRIHumanBrainExp_${registrationpackage}_${size}_CompareTest
        ${Slicer3_EXE} --launch vtkEMSegmentCompareTest ${EMS_SLICER_LAUNCH_ARGUMENTS}
        ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrainExp_T1_aligned.nrrd
                          MIDAS{RESULT_${registrationpackage}_${size}_MRIHumanBrainExp_T1_aligned.nrrd.md5}
        )
    endif(EMSEG_RUN_COMPARE_TESTS)



    # Task MRI-Human-Brain-Parcellation
    add_test( EMSegCL_Task_MRIHumanBrainParcellation_${registrationpackage}_${size}
      ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      ${EMS_SLICER_LAUNCH_ARGUMENTS}
      --verbose --mrmlSceneFileName
      ${EMSegment_SOURCE_DIR}/Tasks/MRI-Human-Brain-Parcellation.mrml
      --targetVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_MRIHumanBrainParcellation_T1.nrrd
      --resultVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${size}_MRIHumanBrainParcellation_T1.nrrd
      --registrationPackage
      ${registrationpackage}
      --registrationAffineType
      -1
      --registrationDeformableType
      -1
      --taskPreProcessingSetting
      ":C1:C0"
      --atlasVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_bg_atlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_ltgm1_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_ltgm2_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_ltgm3_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_ltgm4_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_rtgm1_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_rtgm2_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_rtgm3_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_rtgm4_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_subgm_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_ltwm1_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_ltwm2_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_ltwm3_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_ltwm4_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_rtwm1_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_rtwm2_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_rtwm3_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_rtwm4_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_subwm_combatlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_csf_atlas.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Parcellation/${size}_t1_atlas.nrrd
    )
    set_tests_properties(
      EMSegCL_Task_MRIHumanBrainParcellation_${registrationpackage}_${size}
      PROPERTIES
      FAIL_REGULAR_EXPRESSION
      "ERROR"
      )
    if(EMSEG_RUN_COMPARE_TESTS)
      midas_add_test(
        EMSegCL_Task_MRIHumanBrainParcellation_${registrationpackage}_${size}_CompareTest
        ${Slicer3_EXE} --launch vtkEMSegmentCompareTest ${EMS_SLICER_LAUNCH_ARGUMENTS}
        ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrainParcellation_T1.nrrd
                          MIDAS{RESULT_${registrationpackage}_${size}_MRIHumanBrainParcellation_T1.nrrd.md5}
      )
    endif(EMSEG_RUN_COMPARE_TESTS)
    

    # Task MRI-Human-Brain-Full-Parcellation 
    add_test( EMSegCL_Task_MRIHumanBrainFullParcellation_${registrationpackage}_${size}
      ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      ${EMS_SLICER_LAUNCH_ARGUMENTS}
      --verbose --mrmlSceneFileName
      ${EMSegment_SOURCE_DIR}/Tasks/MRI-Human-Brain-Full-Parcellation.mrml
      --targetVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_MRIHumanBrain_T1_aligned.nrrd
      --resultVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${size}_MRIHumanBrainFullParcellation_T1.nrrd
      --registrationPackage
      ${registrationpackage}
      --registrationAffineType
      -1
      --registrationDeformableType
      -1
      --taskPreProcessingSetting
      ":C1:C0"
      --atlasVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Full-Parcellation/${size}_atlas_skullneck.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Full-Parcellation/${size}_atlas_air.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Full-Parcellation/${size}_atlas_csf.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Full-Parcellation/${size}_atlas_greymatter.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Full-Parcellation/${size}_atlas_whitematter.nrrd,${EMSegment_TESTDATA_DIR}/MRI-Human-Brain-Full-Parcellation/${size}_atlas_t1.nrrd
      )
    set_tests_properties(
      EMSegCL_Task_MRIHumanBrainFullParcellation_${registrationpackage}_${size}
      PROPERTIES
      FAIL_REGULAR_EXPRESSION
      "ERROR"
      )
    if(EMSEG_RUN_COMPARE_TESTS)
      midas_add_test(
        EMSegCL_Task_MRIHumanBrainFullParcellation_${registrationpackage}_${size}_CompareTest
        ${Slicer3_EXE} --launch vtkEMSegmentCompareTest ${EMS_SLICER_LAUNCH_ARGUMENTS}
        ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrainFullParcellation_T1.nrrd
                          MIDAS{RESULT_${registrationpackage}_${size}_MRIHumanBrainFullParcellation_T1.nrrd.md5}
        )
    endif(EMSEG_RUN_COMPARE_TESTS)


    if(NOT EM_Slicer4_FOUND)
    # Task Test-Atlas-Cluster, only for Slicer3
    add_test( EMSegCL_Task_TestAtlasCluster_${registrationpackage}_${size}
      ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      ${EMS_SLICER_LAUNCH_ARGUMENTS}
      --verbose --mrmlSceneFileName
      ${EMSegment_SOURCE_DIR}/Tasks/Test-Atlas-Cluster.mrml
      --targetVolumeFileNames
      ${EMSegment_SOURCE_DIR}/../AtlasCreator/TestData/originals/case62.nrrd
      --registrationPackage
      ${registrationpackage}
      --resultVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${size}_TestAtlasCluster.nrrd
      --taskPreProcessingSetting
      :C1:C0:Eshare/Slicer3/Modules/AtlasCreator/TestData/originals/:Eshare/Slicer3/Modules/AtlasCreator/TestData/segmentations/:E${EMSegment_BINARY_DIR}/ACEMS
      )
    set_tests_properties(
      EMSegCL_Task_TestAtlasCluster_${registrationpackage}_${size}
      PROPERTIES
      FAIL_REGULAR_EXPRESSION
      "ERROR"
      )
    endif(NOT EM_Slicer4_FOUND)

    if (EMSEG_RUN_LOCAL_TESTS)
      # Task cine MRI
      add_test( EMSegCL_Task_ACcineMRI_${registrationpackage}_${size}
        ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
        ${EMS_SLICER_LAUNCH_ARGUMENTS}
        --verbose --mrmlSceneFileName
        ${EMSegment_SOURCE_DIR}/../KilisSandbox/Tasks/AC-cineMR-Ventricles.mrml
        --targetVolumeFileNames
        #      ${EMSegment_SOURCE_DIR}/../KilisSandbox/Tasks/corrected_raw/SARCOIDA1.ED_short.nrrd
        ${EMSegment_SOURCE_DIR}/../KilisSandbox/Tasks/corrected_raw/SARCOIDA1.ED.nii
        --registrationPackage
        ${registrationpackage}
        --resultVolumeFileName
        ${EMSegment_BINARY_DIR}/RESULT_${size}_cineMRI.nrrd
        #      --taskPreProcessingSetting
        #      :C1:C0:E../Slicer3/Modules/AtlasCreator/TestData/originals/:E../Slicer3/Modules/AtlasCreator/TestData/segmentations/:E${EMSegment_BINARY_DIR}/ACEMS
        )
      set_tests_properties(
        EMSegCL_Task_ACcineMRI_${registrationpackage}_${size}
        PROPERTIES
        FAIL_REGULAR_EXPRESSION
        "ERROR"
        )
    endif (EMSEG_RUN_LOCAL_TESTS)


    # Task Non-Human-Primate
    add_test(
      EMSegCL_Task_NonHumanPrimate_${registrationpackage}_${size}
      ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      ${EMS_SLICER_LAUNCH_ARGUMENTS}
      --verbose --mrmlSceneFileName
      ${EMSegment_SOURCE_DIR}/Tasks/Non-Human-Primate.mrml
      --targetVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_NonHumanPrimate_T1.nrrd
      --resultVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${size}_NonHumanPrimate_T1.nrrd
      --taskPreProcessingSetting
      ":C1:C0"
      --registrationPackage
      ${registrationpackage}
      --registrationAffineType
      -1
      --registrationDeformableType
      -1
      --atlasVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_rhip_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_rput_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_rcau_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_lhip_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_lput_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_lcau_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_gm_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_wm_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_csf_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_cb_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_bs_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_rest_prob.nrrd,${EMSegment_TESTDATA_DIR}/Non-Human-Primate/Unbiased/${size}_template.nrrd
      )
    set_tests_properties(
      EMSegCL_Task_NonHumanPrimate_${registrationpackage}_${size}
      PROPERTIES
      FAIL_REGULAR_EXPRESSION
      "ERROR"
      )
    if(EMSEG_RUN_COMPARE_TESTS)
      midas_add_test(
        NAME
        EMSegCL_Task_NonHumanPrimate_${registrationpackage}_${size}_CompareTest
        COMMAND
        ${Slicer3_EXE} --launch vtkEMSegmentCompareTest ${EMS_SLICER_LAUNCH_ARGUMENTS}
        ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_NonHumanPrimate_T1.nrrd
                          MIDAS{RESULT_${registrationpackage}_${size}_NonHumanPrimate_T1.nrrd.md5}
        )
    endif(EMSEG_RUN_COMPARE_TESTS)


    endforeach (registrationpackage ${registrationpackages})

  endforeach (size ${sizes})


if( EMSEG_RUN_LOCAL_TESTS )

  # Task test - CT Hand Bone
  midas_add_test( NAME EMSegCL_TaskCTHandBone_Subject1
    COMMAND ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --mrmlSceneFileName
    ${EMSegment_SOURCE_DIR}/Tasks/CT-Hand-Bone.mrml
    --targetVolumeFileNames
    MIDAS{CT_Hand_Bone_Subject_1.nrrd.md5}
    --resultVolumeFileName
    ${EMSegment_BINARY_DIR}/RESULT_CT_Hand_Bone_Subject_1.nrrd
    )
  set_tests_properties(
    EMSegCL_TaskCTHandBone_Subject1
    PROPERTIES
    FAIL_REGULAR_EXPRESSION
    "ERROR"
    )

  # Task test - CT Hand Bone
  midas_add_test( NAME EMSegCL_TaskCTHandBone_Subject2
    COMMAND ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --mrmlSceneFileName
    ${EMSegment_SOURCE_DIR}/Tasks/CT-Hand-Bone.mrml
    --targetVolumeFileNames
    MIDAS{CT_Hand_Bone_Subject_2.nrrd.md5}
    --resultVolumeFileName
    ${EMSegment_BINARY_DIR}/RESULT_CT_Hand_Bone_Subject_2.nrrd
    )
  set_tests_properties(
    EMSegCL_TaskCTHandBone_Subject1
    PROPERTIES
    FAIL_REGULAR_EXPRESSION
    "ERROR"
    )

  # Task test - CT Hand Bone
  midas_add_test( NAME EMSegCL_TaskCTHandBone_Subject3
    COMMAND ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
    ${EMS_SLICER_LAUNCH_ARGUMENTS}
    --verbose --mrmlSceneFileName
    ${EMSegment_SOURCE_DIR}/Tasks/CT-Hand-Bone.mrml
    --targetVolumeFileNames
    MIDAS{CT_Hand_Bone_Subject_3.nrrd.md5}
    --resultVolumeFileName
    ${EMSegment_BINARY_DIR}/RESULT_CT_Hand_Bone_Subject_3.nrrd
    )
  set_tests_properties(
    EMSegCL_TaskCTHandBone_Subject1
    PROPERTIES
    FAIL_REGULAR_EXPRESSION
    "ERROR"
    )

  add_test( RemoteIOTest2
    ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentRemoteIOTest
    http://slicer.org/EMSegmentUpdates/
    ${EMSegment_TEST_DIR}/RemoteIOTest2sdfsd.hmtl
    )

  # Are we able to download our update file ?
  add_test( RemoteIOTest
    ${Slicer3_EXE} ${WRAPPED_TEST_EXE_PREFIX}/vtkEMSegmentRemoteIOTest
    http://slicer.org/EMSegmentUpdates/
    ${Slicer3_BINARY_DIR}/Testing/Temporary/EMSegmenterUpdates
    )

    
    # Compare results
    # Task MRI-Human-Brain_compare

    set (registrationpackage BRAINS)
    set (size small)
    add_test(
      EMSegCL_Task_MRIHumanBrain_${registrationpackage}_${size}_compare
      ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/EMSegmentCommandLine
      ${EMS_SLICER_LAUNCH_ARGUMENTS}
      --verbose --mrmlSceneFileName
      ${EMSegment_SOURCE_DIR}/Tasks/MRI-Human-Brain.mrml
      --targetVolumeFileNames
      ${EMSegment_TESTDATA_DIR}/MRI-Human-Brain/${size}_MRIHumanBrain_T1_aligned.nrrd
      --resultVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrain_T1_aligned.nrrd
      --resultStandardVolumeFileName
      ${EMSegment_BINARY_DIR}/RESULT_${registrationpackage}_${size}_MRIHumanBrain_T1_aligned_compare.nrrd
      --taskPreProcessingSetting
      ":C1:C0"
      --registrationAffineType
      0
      --registrationDeformableType
      0
      )
    set_tests_properties(
      EMSegCL_Task_MRIHumanBrain_${registrationpackage}_${size}_compare
      PROPERTIES
      FAIL_REGULAR_EXPRESSION
      "ERROR"
      )

endif( EMSEG_RUN_LOCAL_TESTS )



endif(NOT DISABLE_CXX_TESTING)

