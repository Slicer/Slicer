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

# Extra Tests:
# Change this to 1 if you want to run the extra EMSegment tests.  They
# require a testing dataset that is not distributed by default with
# Slicer3.  NB: The extra tests may take a long time and a large
# amount of memory!!!  
SET( RUN_VALGRIND_EMSEGMENT_TESTS 0)
SET( RUN_EXTRA_SHORT_EMSEGMENT_TESTS 0)
SET( RUN_EXTRA_LONG_EMSEGMENT_TESTS 0)

IF( NOT DISABLE_CXX_TESTING )

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

  # The test is a stand-alone executable.  However, the Slicer3
  # launcher is needed to set up shared library paths correctly.
  # We create a prefix for the test executable that accomplishes this
  # wrapping.  
  SET( EM_COMMAND_LINE_TEST_PREFIX ${Slicer3_BINARY_DIR}/Slicer3 --launch ${EXECUTABLE_OUTPUT_PATH})
  IF (WIN32)
    SET( EM_COMMAND_LINE_TEST_PREFIX ${EM_COMMAND_LINE_TEST_PREFIX}/${CMAKE_BUILD_TYPE})
  ENDIF (WIN32)

  ############################################################################
  #
  # command line tests---does the logic work, does it fail elegently
  #
  ############################################################################

  # Test that the segmentation results match what the expected
  # results.  This is a legacy test that should not be removed.
  ADD_TEST(
    vtkEMSegmentBlackBoxSegmentationTest_TutorialDataSmallRead
    ${EM_COMMAND_LINE_TEST_PREFIX}/vtkEMSegmentBlackBoxSegmentationTest
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    ${EMSegment_TUTORIAL_DIR}
    "EMSegment Tutorial Template"
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.mhd
    )

  # Does help work right?
  ADD_TEST(
    EMSegCL_Help
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
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
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --version
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_Version
    PROPERTIES
    PASS_REGULAR_EXPRESSION "version"
    )

  # Does the segmenter work and give the correct answer using the
  # default parameters from the small template scene.
  ADD_TEST(
    EMSegCL_RunDefaultNodes
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --resultVolumeFileName
    ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )
  ADD_TEST(
    EMSegCL_DiffDefaultNodesResult
    ${CMAKE_COMMAND} -E compare_files
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw
    )
  ADD_TEST(
    EMSegCL_RemoveDefaultNodesResult   
    ${CMAKE_COMMAND} -E remove
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )

  # Does the segmenter work and give the correct answer when
  # normalization is turned on?
  ADD_TEST(
    EMSegCL_RunNormalize
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
    --resultVolumeFileName 
    ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )
  ADD_TEST(
    EMSegCL_DiffNormalizeResult
    ${CMAKE_COMMAND} -E compare_files
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw
    )
  ADD_TEST(
    EMSegCL_RemoveNormalizeResult   
    ${CMAKE_COMMAND} -E remove
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )

  # Does the segmenter work and give the correct answer when
  # the parameters node is specified on the command line?
  ADD_TEST(
    EMSegCL_RunSetParameterNode
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --parametersMRMLNodeName 
    EMSegment\ Tutorial\ Template    
    --resultVolumeFileName 
    ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )
  ADD_TEST(
    EMSegCL_DiffSetParameterNodeResult
    ${CMAKE_COMMAND} -E compare_files
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw
    )
  ADD_TEST(
    EMSegCL_RemoveSetParameterNodeResult
    ${CMAKE_COMMAND} -E remove
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )

  # Does the segmenter work and give the correct answer when
  # the target images are specified on the command line?
  ADD_TEST(
    EMSegCL_RunSetTargetImages
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd
    --resultVolumeFileName 
    ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )
  ADD_TEST(
    EMSegCL_DiffSetTargetImagesResult
    ${CMAKE_COMMAND} -E compare_files
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw
    )
  ADD_TEST(
    EMSegCL_RemoveSetTargetImagesResult
    ${CMAKE_COMMAND} -E remove
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )

  # Does the segmenter work and give the correct answer when
  # the result image is specified on the command line?
  ADD_TEST(
    EMSegCL_RunSetResultImage
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --resultVolumeFileName 
    ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )
  ADD_TEST(
    EMSegCL_DiffSetResultImageResult
    ${CMAKE_COMMAND} -E compare_files
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw
    )
  ADD_TEST(
    EMSegCL_RemoveSetResultImageResult
    ${CMAKE_COMMAND} -E remove
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )

  # Does the segmenter work and give the correct answer when
  # the everything is specified on the command line?
  ADD_TEST(
    EMSegCL_RunSetEverything
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --resultVolumeFileName 
    ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    --targetVolumeFileNames
    ${EMSegment_TUTORIAL_DIR}/VolumeData/targetT1Normed_small.mhd,${EMSegment_TUTORIAL_DIR}/VolumeData/targetT2Normed_small.mhd
    )
  ADD_TEST(
    EMSegCL_DiffSetEverythingResult
    ${CMAKE_COMMAND} -E compare_files
    ${EMSegment_TUTORIAL_DIR}/StandardData/StandardSegmentationResult_small.raw
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw
    )
  ADD_TEST(
    EMSegCL_RemoveSetEverythingResult
    ${CMAKE_COMMAND} -E remove
    ${EMSegment_TEST_DIR}/EMSegCL_Result.raw ${EMSegment_TEST_DIR}/EMSegCL_Result.mhd
    )

  # Is multithreading disabled when command line flag is given?
  ADD_TEST(
    EMSegCL_DisableMultithreading
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --dontWriteResults
    --disableMultithreading
    --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_DisableMultithreading
    PROPERTIES
    PASS_REGULAR_EXPRESSION "Multithreading is disabled"
    )

  # Does it fail elegently when a bogus parameter node is specified?
  ADD_TEST(
    EMSegCL_EFBogusParameterNode
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --dontWriteResults
    --mrmlSceneFileName 
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
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose
    --dontWriteResults
    --mrmlSceneFileName 
    /tmp/bogus_file_scene21.mrml
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusMRMLScene
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Error: MRML scene file does not exist.")

  # Does it fail elegently when a bogus target images are specified?
  ADD_TEST(
    EMSegCL_EFBogusTargetImages
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --dontWriteResults
    --mrmlSceneFileName 
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
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --dontWriteResults
    --mrmlSceneFileName 
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
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --dontWriteResults
    --mrmlSceneFileName 
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
  # legacy, eventually remove!!!
  ADD_TEST(
    EMSegCL_EFBogusResultStandardImage
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --dontWriteResults
    --mrmlSceneFileName 
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small.mrml
    --resultStandardVolumeFileName
    ${EMSegment_TEST_DIR}/bogus_file.mhd
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusResultStandardImage
    PROPERTIES
    PASS_REGULAR_EXPRESSION 
    "Error: result standard volume file does not exist")

  ############################################################################
  #
  # MRML---do the logic readers and writers work?
  #
  ############################################################################

  # Make sure the default parameter set has not changed
  ADD_TEST(
    EMSegMRML_WriteDefaultNodes
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --generateEmptyMRMLSceneAndQuit
    ${EMSegment_TEST_DIR}/EMSegMRML_EmptyMRMLScene.mrml
    )
  ADD_TEST(
    EMSegMRML_DiffDefaultNodes
    ${CMAKE_COMMAND} -E compare_files
    ${EMSegment_SOURCE_DIR}/Testing/TestData/DefaultMRMLNodes.mrml
    ${EMSegment_TEST_DIR}/EMSegMRML_EmptyMRMLScene.mrml
    )
  ADD_TEST(
    EMSegMRML_RemoveDefaultNodes
    ${CMAKE_COMMAND} -E remove
    ${EMSegment_TEST_DIR}/EMSegMRML_EmptyMRMLScene.mrml
    )

  # Test that the mrml readers/writers work.
  ADD_TEST(
    EMSegMRML_ReadWriteTutorialData
    ${EM_COMMAND_LINE_TEST_PREFIX}/vtkEMSegmentReadWriteMRMLTest
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
    ${EMSegment_TEST_DIR}/EMSegmentMRML_ReadWriteTutorialScene.mrml
    )
  ADD_TEST(
    EMSegMRML_DiffTutorialData
    ${CMAKE_COMMAND} -E compare_files
    ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
    ${EMSegment_TEST_DIR}/EMSegmentMRML_ReadWriteTutorialScene.mrml
    )
  ADD_TEST(
    EMSegMRML_RemoveTutorialDataOutput
    ${CMAKE_COMMAND} -E remove
    ${EMSegment_TEST_DIR}/EMSegmentMRML_ReadWriteTutorialScene.mrml
    )

  IF ( RUN_VALGRIND_EMSEGMENT_TESTS )
    ADD_TEST(
      EMSegMRML_ValgrindTest
      valgrind --leak-check=full 
      ${EXECUTABLE_OUTPUT_PATH}/EMSegmentCommandLine
      --verbose 
      --mrmlSceneFileName 
      ${EMSegment_TUTORIAL_DIR}/EMSegmentTutorialTemplate_small_normalizationOn.mrml
      --dontWriteResults
      )
  ENDIF ( RUN_VALGRIND_EMSEGMENT_TESTS )

  IF ( RUN_EXTRA_SHORT_EMSEGMENT_TESTS )
    
    # build by hand
    #ADD_TEST(
    #  EMSegCL_RebuildLarge
    #  ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    #  --verbose 
    #  --mrmlSceneFileName 
    #  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/RebuildLarge.mrml
    #  --resultVolumeFileName
    #  /tmp/EMSegCL_RebuildLarge_Result.mhd
    #  --resultStandardVolumeFileName
    #  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_large.mhd
    #  )
    #ADD_TEST(
    #  EMSegMRML_DiffRebuildLarge
    #  diff -s
    #  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_large.raw
    #  /tmp/EMSegCL_RebuildLarge_Result.raw
    #  )
    #SET_TESTS_PROPERTIES(
    #  EMSegMRML_DiffRebuildLarge    
    #  PROPERTIES
    #  PASS_REGULAR_EXPRESSION 
    #  "are identical")

    # be careful, this overwrites data!
    #ADD_TEST(
    #  EMSegCL_DefaultNodesOverwriteResults
    #  ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    #  --verbose 
    #  --mrmlSceneFileName 
    #  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    #  --resultStandardVolumeFileName 
    #  ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
    #  )
    
  ENDIF (RUN_EXTRA_SHORT_EMSEGMENT_TESTS )

  IF ( RUN_EXTRA_LONG_EMSEGMENT_TESTS )
    # test that the segmentation results match what the expected
    # results.
    ADD_TEST(
      vtkEMSegmentBlackBoxSegmentationTest_TutorialDataLarge
      ${EM_COMMAND_LINE_TEST_PREFIX}/vtkEMSegmentBlackBoxSegmentationTest
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_large.mrml
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/
      "EMSegment Tutorial Template"
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_large.mhd
      )

    ADD_TEST(
      EMSegCL_Valgrind
      valgrind ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
      --verbose 
      --dontWriteResults
      --mrmlSceneFileName 
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
      --resultVolumeFileName
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/test_result.mhd    
      --targetVolumeFileNames
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT1Normed_small.mhd,${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT2Normed_small.mhd
      --resultStandardVolumeFileName
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
      )

  ENDIF ( RUN_EXTRA_LONG_EMSEGMENT_TESTS )

ENDIF( NOT DISABLE_CXX_TESTING )
