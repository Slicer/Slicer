
include_directories( ${CLI_SOURCE_DIR} ${RegisterImagesMultiResApplications_SOURCE_DIR} )
link_directories( ${RegisterImagesMultiResApplications_BUILD_DIR} )

set(BASELINE "${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI")
set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

set(RegisterImagesMultiResApplicationsTests_SOURCE
  FixedRotationSimilarity3DTransformTest.cxx
  EulerSimilarity3DTransformTest.cxx
  EulerAnisotropicSimilarity3DTransformTest.cxx
  DecomposedAffine3DTransformTest.cxx
  DownsampleHeuristicsTest.cxx
  ScalingHeuristicsTest.cxx
  ConvertSlicerROIToRegionTest.cxx
  CountSamplesTest.cxx
  )

create_test_sourcelist(RegisterImagesMultiResApplicationsTestsList
                       RegisterImagesMultiResApplicationsTests.cxx
                       ${RegisterImagesMultiResApplicationsTests_SOURCE})

ADD_EXECUTABLE(RegisterImagesMultiResApplicationsTests ${RegisterImagesMultiResApplicationsTestsList} )
TARGET_LINK_LIBRARIES(RegisterImagesMultiResApplicationsTests RegisterImagesMultiResModule)
SET_TARGET_PROPERTIES(RegisterImagesMultiResApplicationsTests PROPERTIES LABELS ${CLP})

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:RegisterImagesMultiResApplicationsTests> ${TESTNAME})
  SET_PROPERTY(TEST ${TESTNAME} PROPERTY LABELS ${CLP})
ENDMACRO()

SIMPLE_TEST( FixedRotationSimilarity3DTransformTest )
SIMPLE_TEST( EulerSimilarity3DTransformTest )
SIMPLE_TEST( EulerAnisotropicSimilarity3DTransformTest )
SIMPLE_TEST( DecomposedAffine3DTransformTest )
SIMPLE_TEST( DownsampleHeuristicsTest )
SIMPLE_TEST( ScalingHeuristicsTest )
SIMPLE_TEST( ConvertSlicerROIToRegionTest )
SIMPLE_TEST( CountSamplesTest )

add_executable(RegisterImagesMultiResTest RegisterImagesMultiResTest.cxx)
target_link_libraries(RegisterImagesMultiResTest RegisterImagesMultiResModule)
SET_TARGET_PROPERTIES(RegisterImagesMultiResTest PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:RegisterImagesMultiResTest>
  --compare   ${BASELINE}/RegisterImagesMultiResTest.nrrd
  ${TEMP}/RegisterImagesMultiResResult.nrrd
  --compareIntensityTolerance 20
  --compareRadiusTolerance 2
  #--compareNumberOfPixelsTolerance 2
  ModuleEntryPoint
  ${MRML_TEST_DATA}/fixed.nrrd
  ${MRML_TEST_DATA}/moving.nrrd
  --resampledImage   ${TEMP}/RegisterImagesMultiResResult.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
