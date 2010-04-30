
configure_file(${BRAINSFit_SOURCE_DIR}/CTestCustom.ctest ${BRAINSFit_BINARY_DIR}/CTestCustom.ctest COPYONLY)

add_executable(BRAINSFitTest BRAINSFitTest.cxx ../BRAINSFitPrimary.cxx)
target_link_libraries(BRAINSFitTest BRAINSCommonLib ITKAlgorithms ITKIO ITKBasicFilters ${OPTIONAL_DEBUG_LINK_LIBRARIES})

set_target_properties(BRAINSFitTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSFit_BINARY_DIR})
set(BRAINSFitTest_EXEC ${BRAINSFit_BINARY_DIR}/BRAINSFitTest)

# The following program was used to generate TestData/test2.nii.gz
# from test.nii.gz
if(MakeMakeXfrmImage)
  add_executable(makexfrmedImage makexfrmedImage.cxx)
  target_link_libraries(makexfrmedImage ITKIO)
endif(MakeMakeXfrmImage)

# The test driver is only needed to help generate new testcases
add_executable(BRAINSFitTestDriver BRAINSFitTestDriver.cxx)
target_link_libraries(BRAINSFitTestDriver ITKAlgorithms ITKIO ITKBasicFilters ${OPTIONAL_DEBUG_LINK_LIBRARIES} )
set_target_properties(BRAINSFitTestDriver PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSFit_BINARY_DIR})

#  if(NOT XXXNO_BRAINSFIT_TESTING)  #When BRAINSFit is a sub project, it should not be tested.
  set(BRAINSFitTestName BRAINSFitTest_AffineRotationMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 131072
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Affine
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_Initializer_RigidRotationNoMasks.mat
    --maskProcessingMode ROI
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/test.mask
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.mask
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
    --debugLevel 50
  )

  set(BRAINSFitTestName BRAINSFitTest_AffineRotationNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 131072
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Affine
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_0.05_${BRAINSFitTestName}.mat
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_AffineScaleMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Affine
    --initializeTransformMode MomentsOn
    --maskProcessingMode ROI
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/test.mask
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/scale.test.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/scale.test.mask
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_AffineScaleNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Affine
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_0.05_${BRAINSFitTestName}.mat
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/scale.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_AffineTranslationMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Affine
    --initializeTransformMode MomentsOn
    --maskProcessingMode ROI
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/test.mask
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/translation.test.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/translation.test.mask
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_AffineTranslationNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Affine
    --initializeTransformMode MomentsOn
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/translation.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )


  set(BRAINSFitTestName BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500,2500,2500,500
    --numberOfHistogramBins 200
    --splineGridSize 7,5,6
    --numberOfSamples 144000
    --translationScale 250
    --minimumStepSize 0.01,0.003,0.001,0.001
    --outputVolumePixelType short
    --maskProcessingMode ROIAUTO
    --initializeTransformMode CenterOfHead
    --transformType Rigid,ScaleVersor3D,Affine,BSpline
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.rescale.rigid.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
    --maxBSplineDisplacement 7.3
  )

  set(BRAINSFitTestName BRAINSFitTest_BSplineOnlyRescaleHeadMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 1500
    --numberOfHistogramBins 200
    --splineGridSize 7,5,6
    --numberOfSamples 144000
    --translationScale 250
    --minimumStepSize 0.01
    --outputVolumePixelType short
    --maskProcessingMode ROIAUTO
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks.mat
    --transformType BSpline
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.rescale.rigid.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
    --debugLevel 10
    --maxBSplineDisplacement 7.3
    --projectedGradientTolerance 1e-4
    --costFunctionConvergenceFactor 1e+9
  )
## NOTE: 7.3 above was computed explicitly through testing.

  set(BRAINSFitTestName BRAINSFitTest_BSplineBSplineRescaleHeadMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 1500
    --numberOfHistogramBins 200
    --splineGridSize 7,5,6
    --numberOfSamples 288000
    --translationScale 250
    --minimumStepSize 0.005
    --outputVolumePixelType short
    --maskProcessingMode ROIAUTO
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_BRAINSFitTest_BSplineOnlyRescaleHeadMasks.mat
    --transformType BSpline
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.rescale.rigid.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
    --maxBSplineDisplacement 7.3
  )

  set(BRAINSFitTestName BRAINSFitTest_BSplineScaleRotationRescaleHeadMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500,2500,500
    --numberOfHistogramBins 200
    --splineGridSize 7,5,6
    --numberOfSamples 144000
    --translationScale 250
    --minimumStepSize 0.01,0.003,0.01
    --outputVolumePixelType short
    --maskProcessingMode ROIAUTO
    --initializeTransformMode CenterOfHead
    --transformType Rigid,ScaleVersor3D,BSpline
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.rescale.rigid.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
    --maxBSplineDisplacement 7.3
  )


  set(BRAINSFitTestName BRAINSFitTest_BSplineScaleRotationHistogramHeadMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500,2500,500
    --numberOfHistogramBins 50
    --numberOfMatchPoints 10
    --histogramMatch
    --splineGridSize 7,5,6
    --numberOfSamples 144000
    --translationScale 250
    --minimumStepSize 0.01,0.003,0.01
    --outputVolumePixelType short
    --maskProcessingMode ROIAUTO
    --initializeTransformMode CenterOfHead
    --transformType Rigid,ScaleVersor3D,BSpline
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.rescale.rigid.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
    --maxBSplineDisplacement 7.3
  )


  set(BRAINSFitTestName BRAINSFitTest_RigidAnisotropicMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
  ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_dbg_splayed.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/ANON0006_20_T1_sag_twisted.nii.gz
    --fixedVolumeTimeIndex 0 --movingVolumeTimeIndex 0
    --minimumStepSize 0.001 --numberOfSamples 100000 --numberOfIterations 1500
    --numberOfHistogramBins 200
    --transformType Rigid --initializeTransformMode MomentsOn --translationScale 1000
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    )

  set(BRAINSFitTestName BRAINSFitTest_RigidMedianRotationNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Rigid
    --initializeTransformMode MomentsOn
    --medianFilterSize 1,1,1
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_RigidRotGeomNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 144000
    --translationScale 500
    --minimumStepSize 0.05,0.0003
    --outputVolumePixelType short
    --transformType Rigid,Rigid
    --initializeTransformMode GeometryOn
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.geom.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_RigidRotaRotaRotNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.05,0.001,0.0000316
    --outputVolumePixelType uchar
    --transformType Rigid,Rigid,Rigid
    --initializeTransformMode CenterOfHead
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_RigidRotationHeadMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Rigid
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_0.05_${BRAINSFitTestName}.mat
    --maskProcessingMode ROIAUTO
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_RigidRotationMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Rigid
    --initializeTransformMode MomentsOn
    --maskProcessingMode ROI
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/test.mask
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.mask
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_RigidRotationNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType Rigid
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_0.05_${BRAINSFitTestName}.mat
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleRotationRescaleHeadMasksNoInit)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500,2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.01,0.003
    --outputVolumePixelType uchar
    --maskProcessingMode ROIAUTO
    --initializeTransformMode CenterOfHead
    --transformType Rigid,ScaleVersor3D
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.rescale.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleSkewVersorRotationMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 11
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 131072
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType ScaleSkewVersor3D
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_Initializer_RigidRotationNoMasks.mat
    --maskProcessingMode ROI
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/test.mask
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.mask
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleSkewVersorRotationNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 11
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 131072
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType ScaleSkewVersor3D
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_0.05_${BRAINSFitTestName}.mat
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleSkewVersorScaleMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1200
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType ScaleSkewVersor3D
    --initializeTransformMode MomentsOn
    --maskProcessingMode ROI
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/test.mask
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/scale.test.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/scale.test.mask
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleSkewVersorScaleNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType ScaleSkewVersor3D
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_0.05_${BRAINSFitTestName}.mat
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/scale.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleTranslationRescaleHeadMasksNoInit)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001,0.0001
    --outputVolumePixelType uchar
    --maskProcessingMode ROIAUTO
    --initializeTransformMode CenterOfHead
    --transformType Rigid,ScaleVersor3D
    --permitParameterVariation 0,0,0,1,1,1
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/translation.rescale.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleTranslationRescaleHeadMasksInit)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.0001
    --outputVolumePixelType uchar
    --maskProcessingMode ROIAUTO
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_BRAINSFitTest_TranslationRescaleHeadMasks.mat
    --transformType ScaleVersor3D
    --permitParameterVariation 0,0,0,1,1,1,1,1,1
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/translation.rescale.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleVersorRotationMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 11
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 131072
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType ScaleVersor3D
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/BRAINSFitTest_Initializer_RigidRotationNoMasks.mat
    --maskProcessingMode ROI
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/test.mask
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.mask
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleVersorRotationNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 11
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 131072
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType ScaleVersor3D
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_0.05_${BRAINSFitTestName}.mat
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/rotation.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleVersorScaleMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType ScaleVersor3D
    --initializeTransformMode MomentsOn
    --maskProcessingMode ROI
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --fixedBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/test.mask
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/scale.test.nii.gz
    --movingBinaryVolume ${BRAINSCommonLib_DATA_DIRS}/scale.test.mask
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_ScaleVersorScaleNoMasks)
  add_test(${BRAINSFitTestName} ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/${BRAINSFitTestName}.result.nii.gz
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --failureExitCode 0 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepSize 0.001
    --outputVolumePixelType uchar
    --transformType ScaleVersor3D
    --initialTransform ${BRAINSCommonLib_DATA_DIRS}/Initializer_0.05_${BRAINSFitTestName}.mat
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/scale.test.nii.gz
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  if(Test_SignedDistanceData)
  add_test(BRAINSFitTest_SignedDistanceData ${BRAINSFitTest_EXEC}
    --compare ${BRAINSCommonLib_DATA_DIRS}/TEST_output.nii.gz ${BRAINSFit_BINARY_DIR}/Testing/TEST_output.nii.gz
    BRAINSFitTest
    --failureExitCode 1
    --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/AtlasBrain_SignedDistance.nii.gz
    --movingVolume ${BRAINSCommonLib_DATA_DIRS}/Unoriented_RawBrain_SignedDistance.nii.gz
    --fixedVolumeTimeIndex 0 --movingVolumeTimeIndex 0
    --minimumStepSize 0.001 --numberOfSamples 75000 --numberOfIterations 1500
    --numberOfHistogramBins 200
    --transformType Rigid --initializeTransformMode MomentsOn --translationScale 1000
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/TEST.mat
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/TEST_output.nii.gz
    )
  endif(Test_SignedDistanceData)

  #  endif(NOT XXXNO_BRAINSFIT_TESTING)  #When BRAINSFit is a sub project, it should not be tested.

