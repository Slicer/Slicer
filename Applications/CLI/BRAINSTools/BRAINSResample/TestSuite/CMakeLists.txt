###  This is the test suite for the BRAINSDemonWarp program.
###  It is intended to test a wide set of image types and image
###  physical space descriptions to ensure that all ITK
###  images are treated properly.


### All test have been pushed to this sub-directory to keep the primary build clean.

add_executable( BRAINSResampleTest  ${BRAINSResample_SOURCE_DIR}/BRAINSResamplePrimary.cxx BRAINSResampleTest.cxx  )
target_link_libraries(BRAINSResampleTest BRAINSCommonLib ITKAlgorithms ITKIO)
set_target_properties(BRAINSResampleTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(BRAINSResampleTest BRAINSResample)

set(BRAINSResample_TESTS ${CMAKE_CURRENT_BINARY_DIR}/BRAINSResampleTest )


add_test(ValidateBRAINSResampleTest4_nii ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.output.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test4.nii.gz
     --compareIntensityTolerance 0
     --compareRadiusTolerance 0
     --compareNumberOfPixelsTolerance 0
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_rotation.input.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.output.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test4.nii.gz
    --pixelType short
    --warpTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.mat
)

add_test(ValidateBRAINSResampleTest5_nii ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/ValidateBRAINSResampleTest5.result.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test5.nii.gz
     --compareIntensityTolerance 10
     --compareRadiusTolerance 0
     --compareNumberOfPixelsTolerance 20
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_rotation.input.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.output.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test5.nii.gz
    --pixelType short
    --interpolationMode WindowedSinc
    --warpTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.mat
)

add_test(ValidateBRAINSResampleTest6_nii ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/ValidateBRAINSResampleTest6.result.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test6.nii.gz
     --compareIntensityTolerance 0
     --compareRadiusTolerance 0
     --compareNumberOfPixelsTolerance 0
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_rotation.input.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.output.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test6.nii.gz
    --pixelType binary
    --warpTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.mat
)

add_test(ValidateBRAINSResampleTest7_nii ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/ValidateBRAINSResampleTest7.result.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test7.nii.gz
     --compareIntensityTolerance 0
     --compareRadiusTolerance 0
     --compareNumberOfPixelsTolerance 0
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test7.nii.gz
    --interpolationMode Linear
    --pixelType uchar
    --warpTransform ${BRAINSCommonLib_DATA_DIRS}/applyWarp_BSpline.mat
)

add_test(ValidateBRAINSResampleTest1_nii ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/applyWarp1.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test1.nii.gz
     --compareNumberOfPixelsTolerance 45
     --compareIntensityTolerance 25
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test1.nii.gz
    --deformationVolume ${BRAINSCommonLib_DATA_DIRS}/OutDefField.nii.gz
    --pixelType short --interpolationMode NearestNeighbor
)
add_test(ValidateBRAINSResampleTest2_nii ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/applyWarp2.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test2.nii.gz
     --compareNumberOfPixelsTolerance 20
     --compareIntensityTolerance 25
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz 
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test2.nii.gz
    --deformationVolume ${BRAINSCommonLib_DATA_DIRS}/OutDefField.nii.gz
    --interpolationMode BSpline --pixelType float
)

add_test(ValidateBRAINSResampleTest3_nii ${BRAINSResample_TESTS}
 --compare
     ${BRAINSCommonLib_DATA_DIRS}/orientedImagesDemons_test.nii.gz
     ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test3.nii.gz
     --compareNumberOfPixelsTolerance 20
     --compareIntensityTolerance 25
   BRAINSResampleTest
    --inputVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_sag_twisted.nii.gz
    --referenceVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz
    --outputVolume ${CMAKE_CURRENT_BINARY_DIR}/applyWarp_test3.nii.gz
    --deformationVolume ${BRAINSCommonLib_DATA_DIRS}/OutDefField_orientedImage.nii.gz
)
