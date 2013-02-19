
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Core/Testing/TestData")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
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

simple_test( FixedRotationSimilarity3DTransformTest )
simple_test( EulerSimilarity3DTransformTest )
simple_test( EulerAnisotropicSimilarity3DTransformTest )
simple_test( DecomposedAffine3DTransformTest )
simple_test( DownsampleHeuristicsTest )
simple_test( ScalingHeuristicsTest )
simple_test( ConvertSlicerROIToRegionTest )
simple_test( CountSamplesTest )

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
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
