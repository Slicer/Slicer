
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
#set(BASELINE "${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI")
set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Core/Testing/TestData")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

set(CLP MultiResolutionAffineRegistration)

create_test_sourcelist(Tests ${CLP}CxxTests.cxx
  FixedRotationSimilarity3DTransformTest.cxx
  EulerSimilarity3DTransformTest.cxx
  EulerAnisotropicSimilarity3DTransformTest.cxx
  DecomposedAffine3DTransformTest.cxx
  DownsampleHeuristicsTest.cxx
  ScalingHeuristicsTest.cxx
  ConvertSlicerROIToRegionTest.cxx
  CountSamplesTest.cxx
  )

add_executable(${CLP}CxxTests ${Tests})
target_link_libraries(${CLP}CxxTests ${CLP}Lib)
set_target_properties(${CLP}CxxTests PROPERTIES LABELS ${CLP})

SIMPLE_TEST( FixedRotationSimilarity3DTransformTest )
SIMPLE_TEST( EulerSimilarity3DTransformTest )
SIMPLE_TEST( EulerAnisotropicSimilarity3DTransformTest )
SIMPLE_TEST( DecomposedAffine3DTransformTest )
SIMPLE_TEST( DownsampleHeuristicsTest )
SIMPLE_TEST( ScalingHeuristicsTest )
SIMPLE_TEST( ConvertSlicerROIToRegionTest )
SIMPLE_TEST( CountSamplesTest )

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare   ${BASELINE}/${CLP}Test.nrrd
  ${TEMP}/${CLP}Result.nrrd
  --compareIntensityTolerance 20
  --compareRadiusTolerance 2
  #--compareNumberOfPixelsTolerance 2
  ModuleEntryPoint
  ${MRML_TEST_DATA}/fixed.nrrd
  ${MRML_TEST_DATA}/moving.nrrd
  --resampledImage   ${TEMP}/${CLP}Result.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
