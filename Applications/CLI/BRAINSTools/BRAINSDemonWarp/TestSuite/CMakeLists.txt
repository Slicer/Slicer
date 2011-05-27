###  This is the test suite for the BRAINSDemonWarp program.
###  It is intended to test a wide set of image types and image
###  physical space descriptions to ensure that all ITK
###  images are treated properly.


### All test have been pushed to this sub-directory to keep the primary build clean.

if(WIN32)
  set(BRAINSWARP_ITK_LIBS "")
  set(BUILD_SUBDIR ${CMAKE_BUILD_TYPE}/)
else(WIN32)
  set(BRAINSWARP_ITK_LIBS ${ITK_LIBRARIES})
endif(WIN32)

add_executable( BRAINSDemonWarpCLPTests BRAINSDemonWarpCLPTests.cxx ${BRAINSDemonWarp_SOURCE_DIR}/BRAINSDemonWarpPrimary.cxx)
target_link_libraries(BRAINSDemonWarpCLPTests BRAINSDemonWarpCommonLib ${BRAINSWARP_ITK_LIBS} ${OPTIONAL_DEBUG_LINK_LIBRARIES})
set_target_properties(BRAINSDemonWarpCLPTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSDemonWarp_BINARY_DIR})
set(BRAINS_Demon_Warp_TESTS ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:BRAINSDemonWarpCLPTests> )

add_executable( VBRAINSDemonWarpCLPTests VBRAINSDemonWarpCLPTests.cxx ${BRAINSDemonWarp_SOURCE_DIR}/VBRAINSDemonWarpPrimary.cxx)
target_link_libraries(VBRAINSDemonWarpCLPTests BRAINSDemonWarpCommonLib ${BRAINSWARP_ITK_LIBS} ${OPTIONAL_DEBUG_LINK_LIBRARIES})
set_target_properties(VBRAINSDemonWarpCLPTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSDemonWarp_BINARY_DIR})
set(VBRAINS_Demon_Warp_TESTS ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:VBRAINSDemonWarpCLPTests> )

include(${BRAINS_CMAKE_HELPER_DIR}/MIDAS.cmake)
set(MIDAS_KEY_DIR "${PROJECT_SOURCE_DIR}/../BRAINSCommonLib/TestData")

#1
midas_add_test(NAME ValidateBRAINSDemonsWarpTest_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{thirionDemons.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/thirionDemons_test.nii.gz
    --compareNumberOfPixelsTolerance 50
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
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
midas_add_test(NAME ValidateFastSymmetricForcesTest_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{fastSymmetricForcesDemons.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/fastSymmetricForcesDemons_test.nii.gz
    --compareNumberOfPixelsTolerance 450
    --compareIntensityTolerance 10
     BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/fastSymmetricForcesDemons_test.nii.gz
    --inputPixelType uchar
    --outputPixelType uchar
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
midas_add_test(NAME ValidateDiffeomorphicTest1_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons1.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons1_test.nii.gz
    --compareNumberOfPixelsTolerance 220
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
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
midas_add_test(NAME ValidateDiffeomorphicTest2_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons2.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons2_test.nii.gz
    --compareNumberOfPixelsTolerance 300
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons2_test.nii.gz
    --inputPixelType ushort
    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume MIDAS{SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz.md5}
    --movingBinaryVolume MIDAS{SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz.md5}
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
midas_add_test(NAME ValidateDiffeomorphicTest3_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons3.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons3_test.nii.gz
    --compareNumberOfPixelsTolerance 6950
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
#    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons3_test.nii.gz
    --inputPixelType int
    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume MIDAS{SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz.md5}
    --movingBinaryVolume MIDAS{SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz.md5}
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
midas_add_test(NAME ValidateDiffeomorphicTest4_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons4.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons4_test.nii.gz
    --compareNumberOfPixelsTolerance 340
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemons4_test.nii.gz
    --inputPixelType int
    --outputPixelType uchar
#    --outputNormalized
    --outputCheckerboardVolume ${BRAINSDemonWarp_BINARY_DIR}/output_Checkborad.nii.gz
    --initializeWithDeformationField MIDAS{InitialDeformationField.nii.gz.md5}
    --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 10,5,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --outputDisplacementFieldPrefix ${BRAINSDemonWarp_BINARY_DIR}/displacement.nii.gz
    --registrationFilterType Diffeomorphic
)

### Need to add more test to get better code coverage of this test suite.  Need to make sure that all the command line options are tested.
midas_add_test(NAME ValidateOrientedImagesTest5_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{ValidateOrientedImagesTest5.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/ValidateOrientedImagesTest5.nii.gz
    --compareNumberOfPixelsTolerance 49800
    --compareIntensityTolerance 25
    BRAINSDemonWarpTest
    --movingVolume MIDAS{ANON0006_20_T1_sag_twisted.nii.gz.md5}
    --fixedVolume MIDAS{ANON0006_20_T1_dbg_splayed.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/ValidateOrientedImagesTest5.nii.gz
    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField_orientedImage.nii.gz
    --inputPixelType uchar
    --outputPixelType uchar
    --smoothDeformationFieldSigma 1.5
    --outputDebug
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 1000,100,25
    --registrationFilterType Diffeomorphic
)

midas_add_test(NAME ValidateVectorBRAINSDemonWarpTest_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{thirionDemons.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/thirionDemons_V_test.nii.gz
    --compareNumberOfPixelsTolerance 50
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/thirionDemons_V_test.nii.gz
#    --inputPixelType short
#    --outputPixelType short
    --outputDebug
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,100,100
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Demons
)


midas_add_test(NAME ValidateVectorFastSymmetricForcesTest_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{fastSymmetricForcesDemons.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/fastSymmetricForcesDemons_V_test.nii.gz
    --compareNumberOfPixelsTolerance 450
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/fastSymmetricForcesDemons_V_test.nii.gz
    --inputPixelType uchar
    --outputPixelType uchar
    --medianFilterSize 1,1,1
    --outputDebug
    --smoothDeformationFieldSigma 1.0
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType FastSymmetricForces
)

midas_add_test(NAME ValidateVectorDiffeomorphicTest1_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons1.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons1_test.nii.gz
    --compareNumberOfPixelsTolerance 220
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5},MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5},MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
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


midas_add_test(NAME ValidateVectorDiffeomorphicTest2_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons2.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons2_test.nii.gz
    --compareNumberOfPixelsTolerance 300
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5},MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5},MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons2_test.nii.gz
    --inputPixelType ushort
    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume MIDAS{SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz.md5}
    --movingBinaryVolume MIDAS{SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz.md5}
    --makeBOBF
    --histogramMatch
    --numberOfHistogramBins 1024    --numberOfMatchPoints 7
    --smoothDeformationFieldSigma 1.0    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 100,50,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
)

midas_add_test(NAME ValidateVectorDiffeomorphicTest3_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons3.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons3_test.nii.gz
    --compareNumberOfPixelsTolerance 6950
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5},MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5},MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
#    --outputDeformationFieldVolume ${VBRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons3_test.nii.gz
    --inputPixelType int
    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume MIDAS{SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz.md5}
    --movingBinaryVolume MIDAS{SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz.md5}
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


midas_add_test(NAME ValidateVectorDiffeomorphicTest4_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons4.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons4_test.nii.gz
    --compareNumberOfPixelsTolerance 340
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5},MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5},MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons4_test.nii.gz
    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --inputPixelType int
    --outputPixelType uchar
#    --outputNormalized
    --outputCheckerboardVolume ${BRAINSDemonWarp_BINARY_DIR}/output_Checkborad.nii.gz
    --initializeWithDeformationField MIDAS{InitialDeformationField.nii.gz.md5}
     --smoothDeformationFieldSigma 1.5
    --numberOfPyramidLevels 3
    --arrayOfPyramidLevelIterations 10,5,5
    --minimumFixedPyramid 4,4,4
    --minimumMovingPyramid 4,4,4
    --registrationFilterType Diffeomorphic
    --outputDisplacementFieldPrefix ${BRAINSDemonWarp_BINARY_DIR}/displacement.nii.gz
)

midas_add_test(NAME ValidateVectorDiffeomorphicTest5_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons5.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons5_test.nii.gz
    --compareNumberOfPixelsTolerance 75
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5},MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5},MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5},MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5},MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
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

midas_add_test(NAME ValidateVectorDiffeomorphicTest6_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{diffeomorphicDemons3.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons6_test.nii.gz
    --compareNumberOfPixelsTolerance 6950
    --compareIntensityTolerance 10
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
#    --outputDeformationFieldVolume ${BRAINSDemonWarp_BINARY_DIR}/OutDefField.nii.gz
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/vectordiffeomorphicDemons6_test.nii.gz
    --inputPixelType int
    --outputPixelType uchar
    --outputNormalized
    --fixedBinaryVolume MIDAS{SUBJ_A_small/SUBJ_A_small_brain_cut_mask.nii.gz.md5}
    --movingBinaryVolume MIDAS{SUBJ_B_small/SUBJ_B_small_brain_cut_mask.nii.gz.md5}
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



midas_add_test(NAME ValidateVectorOrientedImagesTest6_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{ValidateVectorOrientedImagesTest6.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/ValidateVectorOrientedImagesTest6.nii.gz
    --compareNumberOfPixelsTolerance 50000
    --compareIntensityTolerance 25
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{ANON0006_20_T1_sag_twisted.nii.gz.md5},MIDAS{ANON0006_20_T1_sag_twisted.nii.gz.md5}
    --fixedVolume MIDAS{ANON0006_20_T1_dbg_splayed.nii.gz.md5},MIDAS{ANON0006_20_T1_dbg_splayed.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/ValidateVectorOrientedImagesTest6.nii.gz
    --inputPixelType uchar
    --outputPixelType uchar
    --smoothDeformationFieldSigma 1.5
    --outputDebug
    --numberOfPyramidLevels 5
    --arrayOfPyramidLevelIterations 100,30,20,0,0
    --registrationFilterType Diffeomorphic
)

midas_add_test(NAME ValidateVectorOrientedImagesTest7_nii COMMAND ${VBRAINS_Demon_Warp_TESTS}
 --compare
    MIDAS{ValidateVectorOrientedImagesTest7.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/ValidateVectorOrientedImagesTest7.nii.gz
    --compareNumberOfPixelsTolerance 51000
    --compareIntensityTolerance 25
    VBRAINSDemonWarpTest
    --movingVolume MIDAS{ANON0006_20_T1_sag_twisted.nii.gz.md5},MIDAS{ANON0006_20_T1_sag_twisted.nii.gz.md5}
    --fixedVolume MIDAS{ANON0006_20_T1_dbg_splayed.nii.gz.md5},MIDAS{ANON0006_20_T1_dbg_splayed.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/ValidateVectorOrientedImagesTest7.nii.gz
    --inputPixelType uchar
    --outputPixelType uchar
    --smoothDeformationFieldSigma 1.5
    --outputDebug
    --weightFactors 0.8,0.2
    --numberOfPyramidLevels 5
    --arrayOfPyramidLevelIterations 100,30,20,0,0
    --registrationFilterType Diffeomorphic
)

midas_add_test(NAME ValidateDiffeomorphicWithAUTOMask_Test_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
  --compare
    MIDAS{diffeomorphicDemons_AutoMask.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemonsWithAutoMask_test.nii.gz
    --compareNumberOfPixelsTolerance 50
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T2.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T2.nii.gz.md5}
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
    --maskProcessingMode ROIAUTO
    --registrationFilterType Diffeomorphic
)

midas_add_test(NAME ValidateDiffeomorphicWithMask_Test_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
  --compare
    MIDAS{diffeomorphicDemonsWithMask.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/diffeomorphicDemonsWithMask_test.nii.gz
    --compareNumberOfPixelsTolerance 50
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T2.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T2.nii.gz.md5}
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
    --maskProcessingMode ROI
    --fixedBinaryVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T2_mask.nii.gz.md5}
    --movingBinaryVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T2_mask.nii.gz.md5}
    --registrationFilterType Diffeomorphic
)

#Test for initial Transform
midas_add_test(NAME ValidateInitialTransform_Test_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
  --compare
    MIDAS{ValidateInitialTransform_Test.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/ValidateInitialTransform_Test.nii.gz
    --compareNumberOfPixelsTolerance 20
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{rotation.rescale.rigid.nii.gz.md5}
    --fixedVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${BRAINSDemonWarp_BINARY_DIR}/ValidateInitialTransform_Test.nii.gz
    --outputPixelType short
    --outputDebug
    --initializeWithTransform MIDAS{BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks.mat.md5}
    --smoothDeformationFieldSigma 0.0
    --numberOfPyramidLevels 1
    --arrayOfPyramidLevelIterations 1
    --minimumFixedPyramid 1,1,1
    --minimumMovingPyramid 1,1,1
    --registrationFilterType Diffeomorphic
)

#Test for Log Domain
midas_add_test(NAME ValidateLogDemons_Test_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
  --compare
    MIDAS{logDemons.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/logDemons_test.nii.gz
    --compareNumberOfPixelsTolerance 500
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
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

midas_add_test(NAME ValidateSymmetricLogDemons_Test1_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
  --compare
    MIDAS{symmetricLogDemons1.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/symmetricLogDemons_test1.nii.gz
    --compareNumberOfPixelsTolerance 1300
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
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

midas_add_test(NAME ValidateSymmetricLogDemons_Test2_nii COMMAND ${BRAINS_Demon_Warp_TESTS}
  --compare
    MIDAS{symmetricLogDemons2.nii.gz.md5}
    ${BRAINSDemonWarp_BINARY_DIR}/symmetricLogDemons_test2.nii.gz
    --compareNumberOfPixelsTolerance 1300
    --compareIntensityTolerance 10
    BRAINSDemonWarpTest
    --movingVolume MIDAS{SUBJ_B_small/SUBJ_B_small_T1.nii.gz.md5}
    --fixedVolume MIDAS{SUBJ_A_small/SUBJ_A_small_T1.nii.gz.md5}
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
    --initializeWithDeformationField MIDAS{InitialDeformationField.nii.gz.md5}
    --registrationFilterType SymmetricLogDemons
)
