include_directories( ${RegisterImagesMultiResApplications_SOURCE_DIR} )

link_directories( ${RegisterImagesMultiResApplications_BUILD_DIR} )

set(RegisterImagesMultiResApplicationsTests_SOURCE
    FixedRotationSimilarity3DTransformTest.cxx
    EulerSimilarity3DTransformTest.cxx
    EulerAnisotropicSimilarity3DTransformTest.cxx
    DecomposedAffine3DTransformTest.cxx
    DownsampleHeuristicsTest.cxx
    ScalingHeuristicsTest.cxx)

create_test_sourcelist(RegisterImagesMultiResApplicationsTestsList
                       RegisterImagesMultiResApplicationsTests.cxx
                       ${RegisterImagesMultiResApplicationsTests_SOURCE})

ADD_EXECUTABLE(RegisterImagesMultiResApplicationsTests ${RegisterImagesMultiResApplicationsTestsList} )
TARGET_LINK_LIBRARIES(RegisterImagesMultiResApplicationsTests RegisterImagesMultiResModule ${ITK_LIBRARIES})

add_test(FixedRotationSimilarity3DTransformTest
         ${CXX_TEST_PATH}/RegisterImagesMultiResApplicationsTests
         FixedRotationSimilarity3DTransformTest)

add_test(EulerSimilarity3DTransformTest
         ${CXX_TEST_PATH}/RegisterImagesMultiResApplicationsTests
         EulerSimilarity3DTransformTest)

add_test(EulerAnisotropicSimilarity3DTransformTest
         ${CXX_TEST_PATH}/RegisterImagesMultiResApplicationsTests
         EulerAnisotropicSimilarity3DTransformTest)

add_test(DecomposedAffine3DTransformTest
         ${CXX_TEST_PATH}/RegisterImagesMultiResApplicationsTests
         DecomposedAffine3DTransformTest)

add_test(DownsampleHeuristicsTest
         ${CXX_TEST_PATH}/RegisterImagesMultiResApplicationsTests 
         DownsampleHeuristicsTest)

add_test(ScalingHeuristicsTest
         ${CXX_TEST_PATH}/RegisterImagesMultiResApplicationsTests
         ScalingHeuristicsTest)
