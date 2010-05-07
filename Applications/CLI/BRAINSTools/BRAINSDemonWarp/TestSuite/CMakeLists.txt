###  This is the test suite for the BRAINSDemonWarp program.
###  It is intended to test a wide set of image types and image
###  physical space descriptions to ensure that all ITK
###  images are treated properly.


### All test have been pushed to this sub-directory to keep the primary build clean.

if(WIN32)
  set(BRAINSWARP_ITK_LIBS "")
  set(BUILD_SUBDIR ${CMAKE_BUILD_TYPE}/)
else(WIN32)
  set(BRAINSWARP_ITK_LIBS ITKAlgorithms ITKIO)
endif(WIN32)

add_executable( BRAINSDemonWarpCLPTests BRAINSDemonWarpCLPTests.cxx ${BRAINSDemonWarp_SOURCE_DIR}/BRAINSDemonWarpPrimary.cxx)
target_link_libraries(BRAINSDemonWarpCLPTests BRAINSDemonWarpCommonLib ${BRAINSWARP_ITK_LIBS} ${OPTIONAL_DEBUG_LINK_LIBRARIES})
set_target_properties(BRAINSDemonWarpCLPTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSDemonWarp_BINARY_DIR})
set(BRAINS_Demon_Warp_TESTS ${LAUNCH_EXE} ${BRAINSDemonWarp_BINARY_DIR}/${BUILD_SUBDIR}BRAINSDemonWarpCLPTests )

add_executable( VBRAINSDemonWarpCLPTests VBRAINSDemonWarpCLPTests.cxx ${BRAINSDemonWarp_SOURCE_DIR}/VBRAINSDemonWarpPrimary.cxx)
target_link_libraries(VBRAINSDemonWarpCLPTests BRAINSDemonWarpCommonLib ${BRAINSWARP_ITK_LIBS} ${OPTIONAL_DEBUG_LINK_LIBRARIES})
set_target_properties(VBRAINSDemonWarpCLPTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSDemonWarp_BINARY_DIR})
set(VBRAINS_Demon_Warp_TESTS ${LAUNCH_EXE} ${BRAINSDemonWarp_BINARY_DIR}/${BUILD_SUBDIR}VBRAINSDemonWarpCLPTests )

#1
add_test(ValidateBRAINSDemonsWarpTest_nii ${BRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/thirionDemons.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/thirionDemons_test.nii.gz
    --compareNumberOfPixelsTolerance 50
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/thirionDemons_test.nii.gz
#    --inputPixelType short
#    --outputPixelType short
    --outputDebug
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,100,100
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Demons
)

#2
add_test(ValidateFastSymmetricForcesTest_nii ${BRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/fastSymmetricForcesDemons.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/fastSymmetricForcesDemons_test.nii.gz
    --compareNumberOfPixelsTolerance 450
    --compareIntensityTolerance 10
     BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/fastSymmetricForcesDemons_test.nii.gz
    --inputPixelType uchar
    --medianFilterSize 1,1,1
    --outputDebug
    --smoothDeformationFieldSigma 1.0
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType FastSymmetricForces
)

#3
add_test(ValidateDiffeomorphicTest1_nii ${BRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons1.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons1_test.nii.gz
    --compareNumberOfPixelsTolerance 220
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons1_test.nii.gz
    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --inputPixelType short
    --outputPixelType uchar
    --outputNormalized
    --outputDebug
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
)

#4
add_test(ValidateDiffeomorphicTest2_nii ${BRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons2.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons2_test.nii.gz
    --compareNumberOfPixelsTolerance 200
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons2_test.nii.gz
    --inputPixelType ushort
    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz
    --maskProcessingMode BOBF
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.0
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
)

#5
add_test(ValidateDiffeomorphicTest3_nii ${BRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons3.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons3_test.nii.gz
    --compareNumberOfPixelsTolerance 6950
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
#    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons3_test.nii.gz
    --inputPixelType int
    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz
    --maskProcessingMode BOBF
    --smoothDeformationFieldSigma 0.01
    --upFieldSmoothing 1.0
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
)

#6
add_test(ValidateDiffeomorphicTest4_nii ${BRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons4.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons4_test.nii.gz
    --compareNumberOfPixelsTolerance 340
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons4_test.nii.gz
    --inputPixelType int
    --outputPixelType uchar
#    --outputNormalized
    --outputCheckerboardVolume ${BRAINSDemonWarp_BINARY_DIR}/output_Checkborad.nii.gz
    --initializeWithDeformationField ${BRAINSCommonLib_DATA_DIRS}/InitialDeformationField.nii.gz
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 10,5,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
    --outputDisplacementFieldPrefix ${BRAINSDemonWarp_BINARY_DIR}/displacement.nii.gz
)

### Need to add more test to get better code coverage of this test suite.  Need to make sure that all the command line options are tested.
add_test(ValidateOrientedImagesTest5_nii ${BRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/orientedImagesDemons_test.nii.gz
    --compareNumberOfPixelsTolerance 49800
    --compareIntensityTolerance 25
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_sag_twisted.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/orientedImagesDemons_test.nii.gz
    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField_orientedImage.nii.gz
    --inputPixelType uchar
    --smoothDeformationFieldSigma 1.5
    --outputDebug
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 1000,100,25
    --registrationFilterType Diffeomorphic
)

add_test(ValidateVectorBRAINSDemonWarpTest_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/thirionDemons.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/thirionDemons_V_test.nii.gz
    --compareNumberOfPixelsTolerance 50
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/thirionDemons_V_test.nii.gz
#    --inputPixelType short
#    --outputPixelType short
    --outputDebug
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,100,100
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
)


add_test(ValidateVectorFastSymmetricForcesTest_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/fastSymmetricForcesDemons.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/fastSymmetricForcesDemons_V_test.nii.gz
    --compareNumberOfPixelsTolerance 450
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/fastSymmetricForcesDemons_V_test.nii.gz
    --inputPixelType uchar
    --medianFilterSize 1,1,1
    --outputDebug
    --smoothDeformationFieldSigma 1.0
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType FastSymmetricForces
)

add_test(ValidateVectorDiffeomorphicTest1_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons1.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons1_test.nii.gz
    --compareNumberOfPixelsTolerance 220
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons1_test.nii.gz
    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --inputPixelType short
    --outputPixelType uchar
    --outputNormalized
    --outputDebug
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
)


add_test(ValidateVectorDiffeomorphicTest2_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons2.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons2_test.nii.gz
    --compareNumberOfPixelsTolerance 200
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons2_test.nii.gz
    --inputPixelType ushort
    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz
    --makeBOBF
    --histogramMatch
    --numberOfHistogramBins 1024    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.0    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
)

add_test(ValidateVectorDiffeomorphicTest3_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons3.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons3_test.nii.gz
    --compareNumberOfPixelsTolerance 6950
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
#    --outputDeformationFieldVolume ${VBRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons3_test.nii.gz
    --inputPixelType int    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz
    --makeBOBF
    --smoothDeformationFieldSigma 0.01
    --upFieldSmoothing 1.0
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
)


add_test(ValidateVectorDiffeomorphicTest4_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons4.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons4_test.nii.gz
    --compareNumberOfPixelsTolerance 340
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons4_test.nii.gz
    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --inputPixelType int
    --outputPixelType uchar
#    --outputNormalized
    --outputCheckerboardVolume ${BRAINSDemonWarp_BINARY_DIR}/output_Checkborad.nii.gz
    --initializeWithDeformationField ${BRAINSCommonLib_DATA_DIRS}/InitialDeformationField.nii.gz
     --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 10,5,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
    --outputDisplacementFieldPrefix ${BRAINSDemonWarp_BINARY_DIR}/displacement.nii.gz
)

add_test(ValidateVectorDiffeomorphicTest5_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons5.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons5_test.nii.gz
    --compareNumberOfPixelsTolerance 75
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz,${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons5_test.nii.gz
    --inputPixelType short
    --outputPixelType uchar
    --medianFilterSize 1,1,1
    --outputDebug
    --histogramMatch
    --weightFactors 0.5,0.5,0.0
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
)

add_test(ValidateVectorDiffeomorphicTest6_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons3.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons6_test.nii.gz
    --compareNumberOfPixelsTolerance 6950
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
#    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons6_test.nii.gz
    --inputPixelType int
    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz
    --makeBOBF
    --smoothDeformationFieldSigma 0.01
    --upFieldSmoothing 1.0
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
)



add_test(ValidateVectorOrientedImagesTest6_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/orientedVectorImagesDemons_test.nii.gz
    --compareNumberOfPixelsTolerance 50000
    --compareIntensityTolerance 25
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_sag_twisted.nii.gz,${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_sag_twisted.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz,${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/orientedVectorImagesDemons_test.nii.gz
    --inputPixelType uchar
    --smoothDeformationFieldSigma 1.5
    --outputDebug
    --numberOfPyramidLevels 5
    --arrayOfPyramidLevelIterations 100,30,20,0,0
    --registrationFilterType Diffeomorphic
)

add_test(ValidateVectorOrientedImagesTest7_nii ${VBRAINS_Demon_Warp_TESTS}
 --compare
    ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/orientedVectorImagesDemons1_test.nii.gz
    --compareNumberOfPixelsTolerance 51000
    --compareIntensityTolerance 25
    VBRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_sag_twisted.nii.gz,${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_sag_twisted.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz,${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/orientedVectorImagesDemons1_test.nii.gz
    --inputPixelType uchar
    --smoothDeformationFieldSigma 1.5
    --outputDebug
    --weightFactors 0.8,0.2
    --numberOfPyramidLevels 5
    --arrayOfPyramidLevelIterations 100,30,20,0,0
    --registrationFilterType Diffeomorphic
)

add_test(ValidateDiffeomorphicWithAUTOMask_Test_nii ${BRAINS_Demon_Warp_TESTS}
  --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons_AutoMask.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemonsWithAutoMask_test.nii.gz
    --compareNumberOfPixelsTolerance 50
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T2.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T2.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemonsWithAutoMask_test.nii.gz
    --inputPixelType short
    --outputPixelType uchar
    --outputNormalized
    --outputDebug
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
    --maskProcessingMode ROIAUTO
)

add_test(ValidateDiffeomorphicWithMask_Test_nii ${BRAINS_Demon_Warp_TESTS}
  --compare
    ${BRAINSCommonLib_DATA_DIRS}/diffeomorphicDemons_AutoMask.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemonsWithMask_test.nii.gz
    --compareNumberOfPixelsTolerance 50
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T2.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T2.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemonsWithMask_test.nii.gz
    --inputPixelType short
    --outputPixelType uchar
    --outputNormalized
    --outputDebug
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
    --maskProcessingMode ROI
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T2_mask.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T2_mask.nii.gz
)

#Test for initial Transform
add_test(ValidateInitialTransform_Test_nii ${BRAINS_Demon_Warp_TESTS}
  --compare
    ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.output.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemonsWithTransform_test.nii.gz
    --compareNumberOfPixelsTolerance 20
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_rotation.input.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.output.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemonsWithTransform_test.nii.gz
    --outputPixelType uchar
    --outputDebug
    --initializeWithTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_RigidRotationMasks.mat
    --smoothDeformationFieldSigma 0.0
    --numberOfPyramidLevels 1
    --arrayOfPyramidLevelIterations 1
    --minimumFixedPyramid 1,1,1
    --minimumMovingPyramid 1,1,1
    --registrationFilterType Diffeomorphic
)

#Test for Log Domain
add_test(ValidateLogDemons_Test_nii ${BRAINS_Demon_Warp_TESTS}
  --compare
    ${BRAINSCommonLib_DATA_DIRS}/logDemons.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/logDemons_test.nii.gz
    --compareNumberOfPixelsTolerance 0
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/logDemons_test.nii.gz
    --outputDebug
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType LogDemons
)

add_test(ValidateSymmetricLogDemons_Test1_nii ${BRAINS_Demon_Warp_TESTS}
  --compare
    ${BRAINSCommonLib_DATA_DIRS}/symmetricLogDemons1.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/symmetricLogDemons_test1.nii.gz
    --compareNumberOfPixelsTolerance 0
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/symmetricLogDemons_test1.nii.gz
    --outputDebug
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType SymmetricLogDemons
)

add_test(ValidateSymmetricLogDemons_Test2_nii ${BRAINS_Demon_Warp_TESTS}
  --compare
    ${BRAINSCommonLib_DATA_DIRS}/symmetricLogDemons2.nii.gz
    ${BRAINSDemonWarp_BINARY_DIR}/symmetricLogDemons_test2.nii.gz
    --compareNumberOfPixelsTolerance 0
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_B_small/SUBJ_B_small_T1.nii.gz
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/SUBJ_A_small/SUBJ_A_small_T1.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/symmetricLogDemons_test2.nii.gz
    --outputDebug
    --histogramMatch
    --numberOfHistogramBins 1024
    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType SymmetricLogDemons
    --initializeWithDeformationField ${BRAINSCommonLib_DATA_DIRS}/InitialDeformationField.nii.gz
)
