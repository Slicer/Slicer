PROJECT (EMSegmentTesting)

INCLUDE_DIRECTORIES ( 
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${VTK_INCLUDE_DIR}
  )

# Extra Tests:
# Change this to 1 if you want to run the extra EMSegment tests.  They
# require a testing dataset that is not distributed by default with
# Slicer3.  NB: The extra tests may take a long time and a large
# amount of memory!!!  
SET( RUN_EXTRA_EMSEGMENT_TESTS 0)

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

  # The test is a stand-alone executable.  However, the Slicer3
  # launcher is needed to set up shared library paths correctly.
  # We create a prefix for the test executable that accomplishes this
  # wrapping.  
  SET( EM_COMMAND_LINE_TEST_PREFIX ${Slicer3_BINARY_DIR}/Slicer3 --launch ${EXECUTABLE_OUTPUT_PATH})
  IF (WIN32)
    SET( EM_COMMAND_LINE_TEST_PREFIX ${EM_COMMAND_LINE_TEST_PREFIX}/${CMAKE_BUILD_TYPE})
  ENDIF (WIN32)
    
  # test that the segmentation results match what the expected
  # results.
  ADD_TEST(
    vtkEMSegmentBlackBoxSegmentationTest_TutorialDataSmallRead
    ${EM_COMMAND_LINE_TEST_PREFIX}/vtkEMSegmentBlackBoxSegmentationTest
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/
    "EMSegment Tutorial Template"
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
    )

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

  ADD_TEST(
    EMSegCL_EFBogusMRMLScene
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --mrmlSceneFileName 
    /tmp/bogus_file_scene.mrml
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusMRMLScene
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Error: MRML scene file does not exist.")

  ADD_TEST(
    EMSegCL_DefaultNodes
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --resultStandardVolumeFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
    )

  ADD_TEST(
    EMSegCL_DisableMultithreading
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --disableMultithreading
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_DisableMultithreading
    PROPERTIES
    PASS_REGULAR_EXPRESSION "Multithreading is disabled"
    )
  
  ADD_TEST(
    EMSegCL_SpecifiedParameterNode
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --parametersMRMLNodeName 
    EMSegment\ Tutorial\ Template    
    --resultStandardVolumeFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
    )

  ADD_TEST(
    EMSegCL_EFBogusParameterNode
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --parametersMRMLNodeName BogusNodeName  
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusParameterNode
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: no EMSegment parameters found in scene with name ")
    

  ADD_TEST(
    EMSegCL_SpecifiedTargetImages
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT1Normed_small.mhd,${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT2Normed_small.mhd
    --resultStandardVolumeFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
    )

  ADD_TEST(
    EMSegCL_EFBogusTargetImages
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT1Normed_small.mhd,${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT2Normed_small.mhd,bogusFile.mhd
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusTargetImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "Error: target volume file ")

  ADD_TEST(
    EMSegCL_EFTooManyImages
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT1Normed_small.mhd,${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT2Normed_small.mhd,${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT2Normed_small.mhd
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFTooManyImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: Number of input channels")

  ADD_TEST(
    EMSegCL_EFTooFewImages
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --targetVolumeFileNames
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT1Normed_small.mhd
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFTooFewImages
    PROPERTIES
    PASS_REGULAR_EXPRESSION
    "ERROR: Number of input channels")

  ADD_TEST(
    EMSegCL_SpecifiedResultImage
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --resultVolumeFileName
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/test_result.mhd    
    --resultStandardVolumeFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
    )

  ADD_TEST(
    EMSegCL_SpecifiedEverything
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --resultVolumeFileName
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/test_result.mhd    
    --targetVolumeFileNames
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT1Normed_small.mhd,${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/targetT2Normed_small.mhd
    --resultStandardVolumeFileName
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
    )

  ADD_TEST(
    EMSegCL_EFBogusResultStandardImage
    ${EM_COMMAND_LINE_TEST_PREFIX}/EMSegmentCommandLine
    --verbose 
    --mrmlSceneFileName 
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    --resultStandardVolumeFileName
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/VolumeData/bogus_file.mhd
    )
  SET_TESTS_PROPERTIES(
    EMSegCL_EFBogusResultStandardImage
    PROPERTIES
    PASS_REGULAR_EXPRESSION 
    "Error: result standard volume file does not exist")

  IF ( RUN_EXTRA_EMSEGMENT_TESTS )
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

  ENDIF ( RUN_EXTRA_EMSEGMENT_TESTS )

ENDIF( NOT DISABLE_CXX_TESTING )
