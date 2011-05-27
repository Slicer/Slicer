###  This is the test suite for the BRAINSResample program.
###  It is intended to test a wide set of image types and image
###  physical space descriptions to ensure that all ITK
###  images are treated properly.

if (WIN32)
  set(BUILD_SUBDIR ${CMAKE_BUILD_TYPE}/)
endif (WIN32)

### All test have been pushed to this sub-directory to keep the primary build clean.

add_executable( BRAINSResampleTest  ${BRAINSResample_SOURCE_DIR}/BRAINSResamplePrimary.cxx BRAINSResampleTest.cxx  )
target_link_libraries(BRAINSResampleTest BRAINSCommonLib ${ITK_LIBRARIES})
set_target_properties(BRAINSResampleTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(BRAINSResampleTest BRAINSResample)

set(BRAINSResample_TESTS ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:BRAINSResampleTest>)

SETIFEMPTY(MIDAS_REST_URL "http://midas.kitware.com/api/rest" CACHE STRING "The MIDAS server where testing data resides")
include(${BRAINS_CMAKE_HELPER_DIR}/MIDAS.cmake)
set(MIDAS_KEY_DIR "${PROJECT_SOURCE_DIR}/../BRAINSCommonLib/TestData")

## Should provide exactly the same result as BRAINSFitTest_AffineRotationMasks
midas_add_test(NAME ValidateBRAINSResampleTest4_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     MIDAS{BRAINSFitTest_AffineRotationMasks.result.nii.gz.md5}
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test4.nii.gz
     --compareIntensityTolerance 13
     --compareRadiusTolerance 1
     --compareNumberOfPixelsTolerance 1
   BRAINSResampleTest
   --inputVolume MIDAS{rotation.test.nii.gz.md5}
   --referenceVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test4.nii.gz
    --pixelType uchar
    --warpTransform MIDAS{BRAINSFitTest_AffineRotationMasks.mat.md5}
)

## Should provide exactly the same result as BRAINSFit BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks test
midas_add_test(NAME ValidateBRAINSResampleTest5_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     MIDAS{BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks.result.nii.gz.md5}
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test5.nii.gz
     --compareIntensityTolerance 13
     --compareRadiusTolerance 1
     --compareNumberOfPixelsTolerance 80
   BRAINSResampleTest
   --inputVolume MIDAS{rotation.rescale.rigid.nii.gz.md5}
   --referenceVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test5.nii.gz
    --pixelType short
    --warpTransform MIDAS{BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks.mat.md5}
)

midas_add_test(NAME ValidateBRAINSResampleTest6_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     MIDAS{BRAINSFitTest_RigidRotationNoMasks.result.nii.gz.md5}
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test6.nii.gz
     --compareIntensityTolerance 13
     --compareRadiusTolerance 1
     --compareNumberOfPixelsTolerance 1
   BRAINSResampleTest
   --inputVolume MIDAS{rotation.test.nii.gz.md5}
   --referenceVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test6.nii.gz
    --pixelType uchar
    --interpolationMode WindowedSinc
    --warpTransform MIDAS{BRAINSFitTest_RigidRotationNoMasks.mat.md5}
)

midas_add_test(NAME ValidateBRAINSResampleTest7_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     MIDAS{ValidateBRAINSResampleTest7.result.nii.gz.md5}
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test7.nii.gz
     --compareIntensityTolerance 0
     --compareRadiusTolerance 0
     --compareNumberOfPixelsTolerance 0
   BRAINSResampleTest
   --inputVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
   --referenceVolume MIDAS{ValidateBRAINSResampleTest7.result.nii.gz.md5}
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test7.nii.gz
    --interpolationMode Linear
    --pixelType uchar
    --warpTransform MIDAS{applyWarp_BSpline.mat.md5}
)

midas_add_test(NAME ValidateBRAINSResampleTest1_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     MIDAS{applyWarp1.nii.gz.md5}
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test1.nii.gz
     --compareNumberOfPixelsTolerance 45
     --compareIntensityTolerance 25
   BRAINSResampleTest
   --inputVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
   --referenceVolume MIDAS{applyWarp1.nii.gz.md5}
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test1.nii.gz
    --deformationVolume MIDAS{OutDefField.nii.gz.md5}
    --pixelType short --interpolationMode NearestNeighbor
)
midas_add_test(NAME ValidateBRAINSResampleTest2_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     MIDAS{applyWarp2.nii.gz.md5}
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test2.nii.gz
     --compareNumberOfPixelsTolerance 20
     --compareIntensityTolerance 25
   BRAINSResampleTest
   --inputVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
   --referenceVolume MIDAS{applyWarp2.nii.gz.md5}
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test2.nii.gz
    --deformationVolume MIDAS{OutDefField.nii.gz.md5}
    --interpolationMode BSpline --pixelType float
)

midas_add_test(NAME ValidateBRAINSResampleTest3_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     MIDAS{orientedImagesDemons_test.nii.gz.md5}
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test3.nii.gz
     --compareNumberOfPixelsTolerance 20
     --compareIntensityTolerance 25
   BRAINSResampleTest
   --inputVolume MIDAS{ANON0006_20_T1_sag_twisted.nii.gz.md5}
   --referenceVolume MIDAS{orientedImagesDemons_test.nii.gz.md5}
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test3.nii.gz
    --deformationVolume MIDAS{OutDefField_orientedImage.nii.gz.md5}
)

if(0) ##This functionality does not work yet
midas_add_test(NAME ValidateBRAINSResampleInvertTest COMMAND ${BRAINSResample_TESTS}
 --compare
     MIDAS{BRAINSResampleTest_Invert.output.nii.gz.md5}
     ${CMAKE_CURRENT_BINARY_DIR}/InvertTransform_Test.nii.gz
     --compareIntensityTolerance 0
     --compareRadiusTolerance 0
     --compareNumberOfPixelsTolerance 0
   BRAINSResampleTest
   --invertTransform
   --inputVolume MIDAS{BRAINSFitTest_RigidRotationMasks.output.nii.gz.md5}
   --referenceVolume MIDAS{BRAINSFitTest_rotation.input.nii.gz.md5}
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test4.nii.gz
    --pixelType short
    --warpTransform MIDAS{BRAINSFitTest_RigidRotationMasks.mat.md5}
)
endif(0)

