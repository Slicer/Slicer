include_directories( ${RegisterImagesMultiResApplications_SOURCE_DIR} )

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
    CountSamplesTest.cxx)

create_test_sourcelist(RegisterImagesMultiResApplicationsTestsList
                       RegisterImagesMultiResApplicationsTests.cxx
                       ${RegisterImagesMultiResApplicationsTests_SOURCE})

ADD_EXECUTABLE(RegisterImagesMultiResApplicationsTests ${RegisterImagesMultiResApplicationsTestsList} )
TARGET_LINK_LIBRARIES(RegisterImagesMultiResApplicationsTests RegisterImagesMultiResModule)

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:RegisterImagesMultiResApplicationsTests> ${TESTNAME})
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

add_test(RegisterImagesMultiResTest ${LAUNCH_EXE} RegisterImagesMultiResTest
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
