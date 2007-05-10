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
  SET( EM_COMMAND_LINE_TEST_PREFIX ${Slicer3_BINARY_DIR}/Slicer3 --launch ${EXECUTABLE_OUTPUT_PATH}/\${CTEST_CONFIGURATION_TYPE}/)
    
  # test that the segmentation results match what the expected
  # results.
  ADD_TEST(
    vtkEMSegmentBlackBoxSegmentationTest_TutorialDataSmallRead
    ${EM_COMMAND_LINE_TEST_PREFIX}vtkEMSegmentBlackBoxSegmentationTest
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/
    "EMSegmentTutorialTemplate"
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
    )

  IF ( RUN_EXTRA_EMSEGMENT_TESTS )
    # test that the segmentation results match what the expected
    # results.
    ADD_TEST(
      vtkEMSegmentBlackBoxSegmentationTest_TutorialDataLarge
      ${EM_COMMAND_LINE_TEST_PREFIX}vtkEMSegmentBlackBoxSegmentationTest
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_large.mrml
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/
      "EMSegment Tutorial Template"
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_large.mhd
      )
  ENDIF ( RUN_EXTRA_EMSEGMENT_TESTS )

ENDIF( NOT DISABLE_CXX_TESTING )
