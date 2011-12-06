
include_directories( ${CLI_SOURCE_DIR} ${RegisterImagesMultiResApplications_SOURCE_DIR} )
link_directories( ${RegisterImagesMultiResApplications_BUILD_DIR} )

set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
#set(BASELINE "${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI")
set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Core/Testing/TestData")
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

add_executable(RegisterImagesMultiResApplicationsTests ${RegisterImagesMultiResApplicationsTestsList} )
target_link_libraries(RegisterImagesMultiResApplicationsTests RegisterImagesMultiResModule)
set_target_properties(RegisterImagesMultiResApplicationsTests PROPERTIES LABELS ${CLP})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:RegisterImagesMultiResApplicationsTests> ${TESTNAME} ${ARGN})
  set_property(TEST ${TESTNAME} PROPERTY LABELS ${CLP})
endmacro()

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
set_target_properties(RegisterImagesMultiResTest PROPERTIES LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
