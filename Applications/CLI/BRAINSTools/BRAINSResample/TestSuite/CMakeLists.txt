###  This is the test suite for the BRAINSResample program.
###  It is intended to test a wide set of image types and image
###  physical space descriptions to ensure that all ITK
###  images are treated properly.

if (WIN32)
  set(BUILD_SUBDIR ${CMAKE_BUILD_TYPE}/)
endif (WIN32)

### All test have been pushed to this sub-directory to keep the primary build clean.

add_executable( BRAINSResampleTest  ${BRAINSResample_SOURCE_DIR}/BRAINSResamplePrimary.cxx BRAINSResampleTest.cxx  )
target_link_libraries(BRAINSResampleTest BRAINSCommonLib ITKAlgorithms ITKIO)
set_target_properties(BRAINSResampleTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(BRAINSResampleTest BRAINSResample)

set(BRAINSResample_TESTS ${LAUNCH_EXE} $<TARGET_FILE:BRAINSResampleTest>)


## Should provide exactly the same result as BRAINSFitTest_AffineRotationMasks
add_test(NAME ValidateBRAINSResampleTest4_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_AffineRotationMasks.result.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test4.nii.gz
     --compareIntensityTolerance 13
     --compareRadiusTolerance 1
     --compareNumberOfPixelsTolerance 1
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test4.nii.gz
    --pixelType uchar
    --warpTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_AffineRotationMasks.mat
)

## Should provide exactly the same result as BRAINSFit BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks test
add_test(NAME ValidateBRAINSResampleTest5_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks.result.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test5.nii.gz
     --compareIntensityTolerance 13
     --compareRadiusTolerance 1
     --compareNumberOfPixelsTolerance 80
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.rescale.rigid.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test5.nii.gz
    --pixelType short
    --warpTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks.mat
)

add_test(NAME ValidateBRAINSResampleTest6_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationNoMasks.result.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test6.nii.gz
     --compareIntensityTolerance 13
     --compareRadiusTolerance 1
     --compareNumberOfPixelsTolerance 1
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test6.nii.gz
    --pixelType uchar
    --interpolationMode WindowedSinc
    --warpTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationNoMasks.mat
)

add_test(NAME ValidateBRAINSResampleTest7_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/ValidateBRAINSResampleTest7.result.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test7.nii.gz
     --compareIntensityTolerance 0
     --compareRadiusTolerance 0
     --compareNumberOfPixelsTolerance 0
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/ValidateBRAINSResampleTest7.result.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test7.nii.gz
    --interpolationMode Linear
    --pixelType uchar
    --warpTransform ${BRAINSCommonLib_DATA_DIRS}/applyWarp_BSpline.mat
)

add_test(NAME ValidateBRAINSResampleTest1_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/applyWarp1.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test1.nii.gz
     --compareNumberOfPixelsTolerance 45
     --compareIntensityTolerance 25
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/applyWarp1.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test1.nii.gz
    --deformationVolume ${BRAINSCommonLib_DATA_DIRS}/OutDefField.nii.gz
    --pixelType short --interpolationMode NearestNeighbor
)
add_test(NAME ValidateBRAINSResampleTest2_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/applyWarp2.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test2.nii.gz
     --compareNumberOfPixelsTolerance 20
     --compareIntensityTolerance 25
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz 
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/applyWarp2.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test2.nii.gz
    --deformationVolume ${BRAINSCommonLib_DATA_DIRS}/OutDefField.nii.gz
    --interpolationMode BSpline --pixelType float
)

add_test(NAME ValidateBRAINSResampleTest3_nii COMMAND ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/orientedImagesDemons_test.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test3.nii.gz
     --compareNumberOfPixelsTolerance 20
     --compareIntensityTolerance 25
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_sag_twisted.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/orientedImagesDemons_test.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test3.nii.gz
    --deformationVolume ${BRAINSCommonLib_DATA_DIRS}/OutDefField_orientedImage.nii.gz
)

if(0) ##This functionality does not work yet
add_test(NAME ValidateBRAINSResampleInvertTest COMMAND ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/BRAINSResampleTest_Invert.output.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/InvertTransform_Test.nii.gz
     --compareIntensityTolerance 0
     --compareRadiusTolerance 0
     --compareNumberOfPixelsTolerance 0
   BRAINSResampleTest
   --invertTransform
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.output.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_rotation.input.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test4.nii.gz
    --pixelType short
    --warpTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.mat
)
endif(0)

