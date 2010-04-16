include_directories( ${RegisterImagesMultiResApplications_SOURCE_DIR} )

link_directories( ${RegisterImagesMultiResApplications_BUILD_DIR} )

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


set(RegisterImagesMultiRes_TESTS ${CXX_TEST_PATH}/RegisterImagesMultiResApplicationsTests)
if(WIN32)
  set(RegisterImagesMultiRes_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/RegisterImagesMultiResApplicationsTests)  
endif(WIN32)

add_test(FixedRotationSimilarity3DTransformTest
         ${LAUNCH_EXE} ${RegisterImagesMultiRes_TESTS}
         FixedRotationSimilarity3DTransformTest)

add_test(EulerSimilarity3DTransformTest
         ${LAUNCH_EXE} ${RegisterImagesMultiRes_TESTS}
         EulerSimilarity3DTransformTest)

add_test(EulerAnisotropicSimilarity3DTransformTest
         ${LAUNCH_EXE} ${RegisterImagesMultiRes_TESTS}
         EulerAnisotropicSimilarity3DTransformTest)

add_test(DecomposedAffine3DTransformTest
         ${LAUNCH_EXE} ${RegisterImagesMultiRes_TESTS}
         DecomposedAffine3DTransformTest)

add_test(DownsampleHeuristicsTest
         ${LAUNCH_EXE} ${RegisterImagesMultiRes_TESTS} 
         DownsampleHeuristicsTest)

add_test(ScalingHeuristicsTest
         ${LAUNCH_EXE} ${RegisterImagesMultiRes_TESTS}
         ScalingHeuristicsTest)

add_test(ConvertSlicerROIToRegionTest
         ${LAUNCH_EXE} ${RegisterImagesMultiRes_TESTS}
         ConvertSlicerROIToRegionTest)
  
add_test(CountSamplesTest
         ${LAUNCH_EXE} ${RegisterImagesMultiRes_TESTS}
         CountSamplesTest)
