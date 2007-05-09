PROJECT (EMSegmentTesting)

INCLUDE_DIRECTORIES ( 
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${VTK_INCLUDE_DIR}
  )

# Change this to 1 if you want to run the extra EMSegment tests.  They
# require a testing dataset that is not distributed by default with
# Slicer3.  NB: The extra tests may take a long time and a large
# amount of memory!!!  
SET( RUN_EXTRA_EMSEGMENT_TESTS 0)

IF( NOT DISABLE_CXX_TESTING )

  INCLUDE(${CMAKE_ROOT}/Modules/Dart.cmake)
  ENABLE_TESTING()

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

  IF(WIN32)
    SET( EM_COMMAND_LINE_TEST_PREFIX ${Slicer3_BINARY_DIR}/Slicer3.exe --launch ${EXECUTABLE_OUTPUT_PATH}/\${CTEST_CONFIGURATION_TYPE}/)
  ELSE(WIN32)
    SET( EM_COMMAND_LINE_TEST_PREFIX ${EXECUTABLE_OUTPUT_PATH}/)
  ENDIF(WIN32)
    
  ADD_TEST(
    vtkEMSegmentBlackBoxSegmentationTest_TutorialDataSmallRead
    ${EM_COMMAND_LINE_TEST_PREFIX}vtkEMSegmentBlackBoxSegmentationTest
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_small.mrml
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/
    "EMSegment Tutorial Template"
    ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_small.mhd
    )

  IF ( RUN_EXTRA_EMSEGMENT_TESTS )
    ADD_TEST(
      vtkEMSegmentBlackBoxSegmentationTest_TutorialDataLarge
      ${EXECUTABLE_OUTPUT_PATH}/vtkEMSegmentBlackBoxSegmentationTest
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/EMSegmentTutorialTemplate_large.mrml
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/
      "EMSegment Tutorial Template"
      ${EMSegment_SOURCE_DIR}/Testing/TestData/TutorialTest/StandardData/StandardSegmentationResult_large.mhd
      )
  ENDIF ( RUN_EXTRA_EMSEGMENT_TESTS )

ENDIF( NOT DISABLE_CXX_TESTING )
