
if (WIN32)
  set(BUILD_SUBDIR ${CMAKE_BUILD_TYPE}/)
endif (WIN32)

configure_file(${BRAINSFit_SOURCE_DIR}/CTestCustom.ctest ${BRAINSFit_BINARY_DIR}/CTestCustom.ctest COPYONLY)

add_executable(BRAINSFitTest BRAINSFitTest.cxx ${BRAINSFit_SOURCE_DIR}/BRAINSFitPrimary.cxx)
target_link_libraries(BRAINSFitTest BRAINSCommonLib ${ITK_LIBRARIES} ${OPTIONAL_DEBUG_LINK_LIBRARIES})
set_target_properties(BRAINSFitTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSFit_BINARY_DIR})
add_dependencies(BRAINSFitTest BRAINSFit)

if(COMPILE_TEST_GENERATION_PROGRAMS)  ## These were programs needed to create the test suite, but are not needed for actual testing!
  # The following program was used to generate TestData/test2.nii.gz
  # from test.nii.gz
  if(MakeMakeXfrmImage)
    add_executable(makexfrmedImage makexfrmedImage.cxx)
    target_link_libraries(makexfrmedImage ${ITK_LIBRARIES})
  endif(MakeMakeXfrmImage)

  if(BRAINSFitTestDriver)
    # The test driver is only needed to help generate new testcases
    add_executable(BRAINSFitTestDriver BRAINSFitTestDriver.cxx)
    target_link_libraries(BRAINSFitTestDriver ${ITK_LIBRARIES}
      ${OPTIONAL_DEBUG_LINK_LIBRARIES} )
    set_target_properties(BRAINSFitTestDriver PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSFit_BINARY_DIR})
  endif()
endif(COMPILE_TEST_GENERATION_PROGRAMS)

include(${BRAINS_CMAKE_HELPER_DIR}/MIDAS.cmake)
## Needed to set this for Slicer3 to find the test data directory
set(BRAINSCommonLib_DATA_DIRS "${PROJECT_SOURCE_DIR}/../BRAINSCommonLib/TestData")
set(MIDAS_KEY_DIR "${BRAINSCommonLib_DATA_DIRS}")

set(BRAINSFitTestName BRAINSFitTest_AffineRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Affine
  --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --debugLevel 50
)

set(BRAINSFitTestName BRAINSFitTest_AffineRotationNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Affine
  --initialTransform MIDAS{Initializer_0.05_${BRAINSFitTestName}.mat.md5}
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_AffineScaleMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Affine
  --initializeTransformMode useMomentsAlign
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{scale.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{scale.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_AffineScaleNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Affine
  --initialTransform MIDAS{Initializer_0.05_${BRAINSFitTestName}.mat.md5}
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{scale.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_AffineTranslationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MSE
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Affine
  --initializeTransformMode useMomentsAlign
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{translation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{translation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_AffineTranslationNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MSE
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Affine
  --initializeTransformMode useMomentsAlign
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{translation.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)


set(BRAINSFitTestName BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500,2500,2500,500
  --numberOfHistogramBins 200
  --splineGridSize 7,5,6
  --numberOfSamples 144000
  --translationScale 250
  --minimumStepLength 0.01,0.003,0.001,0.001
  --outputVolumePixelType short
  --maskProcessingMode ROIAUTO
  --initializeTransformMode useCenterOfHeadAlign
  --transformType Rigid,ScaleVersor3D,Affine,BSpline
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.rescale.rigid.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --maxBSplineDisplacement 7.3
)

set(BRAINSFitTestName BRAINSFitTest_BSplineOnlyRescaleHeadMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 1500
  --numberOfHistogramBins 200
  --splineGridSize 7,5,6
  --numberOfSamples 144000
  --translationScale 250
  --minimumStepLength 0.01
  --outputVolumePixelType short
  --maskProcessingMode ROIAUTO
  --initialTransform MIDAS{Initializer_BRAINSFitTest_BSplineAnteScaleRotationRescaleHeadMasks.mat.md5}
  --transformType BSpline
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.rescale.rigid.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --debugLevel 10
  --maxBSplineDisplacement 7.3
  --projectedGradientTolerance 1e-4
  --costFunctionConvergenceFactor 1e+9
)
## NOTE: 7.3 above was computed explicitly through testing.

set(BRAINSFitTestName BRAINSFitTest_BSplineBSplineRescaleHeadMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 1500
  --numberOfHistogramBins 200
  --splineGridSize 7,5,6
  --numberOfSamples 288000
  --translationScale 250
  --minimumStepLength 0.005
  --outputVolumePixelType short
  --maskProcessingMode ROIAUTO
  --initialTransform MIDAS{Initializer_BRAINSFitTest_BSplineOnlyRescaleHeadMasks.mat.md5}
  --transformType BSpline
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.rescale.rigid.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --maxBSplineDisplacement 7.3
)

set(BRAINSFitTestName BRAINSFitTest_BSplineScaleRotationRescaleHeadMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500,2500,500
  --numberOfHistogramBins 200
  --splineGridSize 7,5,6
  --numberOfSamples 144000
  --translationScale 250
  --minimumStepLength 0.01,0.003,0.01
  --outputVolumePixelType short
  --maskProcessingMode ROIAUTO
  --initializeTransformMode useCenterOfHeadAlign
  --transformType Rigid,ScaleVersor3D,BSpline
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.rescale.rigid.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --maxBSplineDisplacement 7.3
)


set(BRAINSFitTestName BRAINSFitTest_BSplineScaleRotationHistogramHeadMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500,2500,500
  --numberOfHistogramBins 50
  --numberOfMatchPoints 10
  --histogramMatch
  --splineGridSize 7,5,6
  --numberOfSamples 144000
  --translationScale 250
  --minimumStepLength 0.01,0.003,0.01
  --outputVolumePixelType short
  --maskProcessingMode ROIAUTO
  --initializeTransformMode useCenterOfHeadAlign
  --transformType Rigid,ScaleVersor3D,BSpline
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.rescale.rigid.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --maxBSplineDisplacement 7.3
)


set(BRAINSFitTestName BRAINSFitTest_RigidAnisotropicMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --fixedVolume MIDAS{ANON0006_20_T1_dbg_splayed.nii.gz.md5}
  --movingVolume MIDAS{ANON0006_20_T1_sag_twisted.nii.gz.md5}
  --fixedVolumeTimeIndex 0 --movingVolumeTimeIndex 0
  --minimumStepLength 0.001 --numberOfSamples 100000 --numberOfIterations 1500
  --numberOfHistogramBins 200
  --transformType Rigid --initializeTransformMode useMomentsAlign --translationScale 1000
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  )

set(BRAINSFitTestName BRAINSFitTest_RigidMedianRotationNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Rigid
  --initializeTransformMode useMomentsAlign
  --medianFilterSize 1,1,1
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_RigidRotGeomNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 144000
  --translationScale 500
  --minimumStepLength 0.05,0.0050
  --outputVolumePixelType short
  --transformType Rigid,Rigid
  --initializeTransformMode useGeometryAlign
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.geom.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_RigidRotaRotaRotNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.05,0.001,0.00075
  --outputVolumePixelType uchar
  --transformType Rigid,Rigid,Rigid
  --initializeTransformMode useCenterOfHeadAlign
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_RigidRotationHeadMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Rigid
  --initialTransform MIDAS{Initializer_0.05_${BRAINSFitTestName}.mat.md5}
  --maskProcessingMode ROIAUTO
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_RigidRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Rigid
  --initializeTransformMode useMomentsAlign
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_RigidRotationNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Rigid
  --interpolationMode WindowedSinc
  --initialTransform MIDAS{Initializer_0.05_${BRAINSFitTestName}.mat.md5}
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)
set(BRAINSFitTestName BRAINSFitTest_RigidRotationNoMasksRiginInPlaceInterp)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Rigid
  --interpolationMode RigidInPlace
  --initialTransform MIDAS{Initializer_0.05_${BRAINSFitTestName}.mat.md5}
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)


set(BRAINSFitTestName BRAINSFitTest_ScaleRotationRescaleHeadMasksNoInit)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500,2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.01,0.003
  --outputVolumePixelType uchar
  --maskProcessingMode ROIAUTO
  --initializeTransformMode useCenterOfHeadAlign
  --useRigid --useScaleVersor3D
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.rescale.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleSkewVersorRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 11
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleSkewVersor3D
  --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleSkewVersorRotationNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 11
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleSkewVersor3D
  --initialTransform MIDAS{Initializer_0.05_${BRAINSFitTestName}.mat.md5}
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleSkewVersorScaleMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1200
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleSkewVersor3D
  --initializeTransformMode useMomentsAlign
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{scale.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{scale.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleSkewVersorScaleNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleSkewVersor3D
  --initialTransform MIDAS{Initializer_0.05_${BRAINSFitTestName}.mat.md5}
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{scale.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleTranslationRescaleHeadMasksNoInit)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001,0.0001
  --outputVolumePixelType uchar
  --maskProcessingMode ROIAUTO
  --initializeTransformMode useCenterOfHeadAlign
  --transformType Rigid,ScaleVersor3D
  --permitParameterVariation 0,0,0,1,1,1
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{translation.rescale.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleTranslationRescaleHeadMasksInit)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --maskProcessingMode ROIAUTO
  --initialTransform MIDAS{Initializer_BRAINSFitTest_TranslationRescaleHeadMasks.mat.md5}
  --transformType ScaleVersor3D
  --permitParameterVariation 0,0,0,1,1,1,1,1,1
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{translation.rescale.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleVersorRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 11
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleVersor3D
  --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleVersorRotationNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 11
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleVersor3D
  --initialTransform MIDAS{Initializer_0.05_${BRAINSFitTestName}.mat.md5}
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleVersorScaleMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleVersor3D
  --initializeTransformMode useMomentsAlign
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{scale.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{scale.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_ScaleVersorScaleNoMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleVersor3D
  --initialTransform MIDAS{Initializer_0.05_${BRAINSFitTestName}.mat.md5}
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{scale.test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

if(Test_SignedDistanceData)
midas_add_test(NAME BRAINSFitTest_SignedDistanceData COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{TEST_output.nii.gz.md5} ${BRAINSFit_BINARY_DIR}/Testing/TEST_output.nii.gz
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode 1
  --fixedVolume MIDAS{AtlasBrain_SignedDistance.nii.gz.md5}
  --movingVolume MIDAS{Unoriented_RawBrain_SignedDistance.nii.gz.md5}
  --fixedVolumeTimeIndex 0 --movingVolumeTimeIndex 0
  --minimumStepLength 0.001 --numberOfSamples 75000 --numberOfIterations 1500
  --numberOfHistogramBins 200
  --transformType Rigid --initializeTransformMode useMomentsAlign --translationScale 1000
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/TEST.mat
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/TEST_output.nii.gz
  )
endif(Test_SignedDistanceData)

set(BRAINSFitTestName BRAINSFitTest_BSplineScaleRotationHeadMasksUShort)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 2505
  --compareRadiusTolerance 1
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --backgroundFillValue 10000
  --scaleOutputValues
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500,2500,500
  --splineGridSize 7,5,6
  --numberOfSamples 144000
  --translationScale 250
  --minimumStepLength 0.01,0.003,0.01
  --outputVolumePixelType ushort
  --maskProcessingMode ROIAUTO
  --initializeTransformMode useCenterOfHeadAlign
  --transformType Rigid,ScaleVersor3D,BSpline
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotationUShort.rescale.rigid.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --maxBSplineDisplacement 7.3
)

set(BRAINSFitTestName BRAINSFitTest_BSplineScaleRotationHistogramHeadMasksUShort)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MMI
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500,2500,500
  --numberOfHistogramBins 50
  --numberOfMatchPoints 10
  --histogramMatch
  --splineGridSize 7,5,6
  --numberOfSamples 144000
  --translationScale 250
  --minimumStepLength 0.01,0.003,0.01
  --outputVolumePixelType short
  --maskProcessingMode ROIAUTO
  --initializeTransformMode useCenterOfHeadAlign
  --transformType Rigid,ScaleVersor3D,BSpline
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{rotationUShort.rescale.rigid.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --maxBSplineDisplacement 7.3
)

set(BRAINSFitTestName BRAINSFitTest_NCMetricBrainToItself)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{test.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric NC
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 25
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleVersor3D
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_NCAffineRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric NC
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Affine
  --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --debugLevel 50
)

set(BRAINSFitTestName BRAINSFitTest_NCScaleSkewVersorRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 11
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric NC
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleSkewVersor3D
  --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)


set(BRAINSFitTestName BRAINSFitTest_MSEMetricBrainToItself)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{test.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MSE
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 25
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleVersor3D
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_MSEAffineRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MSE
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Affine
  --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --debugLevel 50
)

set(BRAINSFitTestName BRAINSFitTest_MSEScaleSkewVersorRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 11
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MSE
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleSkewVersor3D
  --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_MCMetricBrainToItself)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{test.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MC
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 25
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Rigid --initializeTransformMode useMomentsAlign
  --fixedVolume MIDAS{test.nii.gz.md5}
  --movingVolume MIDAS{test.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_MCMetricMaskBrainToItself)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{testMask.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.testMask.nii.gz
  --compareIntensityTolerance 9
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MC
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 25
  --numberOfHistogramBins 200
  --numberOfSamples 72000
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Rigid --initializeTransformMode useMomentsAlign
  --fixedVolume MIDAS{testMask.nii.gz.md5}
  --movingVolume MIDAS{testMask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.testMask.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

set(BRAINSFitTestName BRAINSFitTest_MCAffineRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 7
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 777
  BRAINSFitTest
  --costMetric MC
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType Affine
  --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  --debugLevel 50
)

set(BRAINSFitTestName BRAINSFitTest_MCScaleSkewVersorRotationMasks)
midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --compareIntensityTolerance 11
  --compareRadiusTolerance 0
  --compareNumberOfPixelsTolerance 1000
  BRAINSFitTest
  --costMetric MC
  --failureExitCode -1 --writeTransformOnFailure
  --numberOfIterations 2500
  --numberOfHistogramBins 200
  --numberOfSamples 131072
  --translationScale 250
  --minimumStepLength 0.001
  --outputVolumePixelType uchar
  --transformType ScaleSkewVersor3D
  --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
  --maskProcessingMode ROI
  --fixedVolume MIDAS{test.nii.gz.md5}
  --fixedBinaryVolume MIDAS{test_mask.nii.gz.md5}
  --movingVolume MIDAS{rotation.test.nii.gz.md5}
  --movingBinaryVolume MIDAS{rotation.test_mask.nii.gz.md5}
  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
)

# Start of broken metric tests
# Add ", KS (Kappa Statistic for binary images with voxel value=255), MRSD (Mean Reciprocal Square Difference), MIH (Mutual Information Histogram), GD (Gradient Difference), CCH (Correlation Coefficient Histogram), MSEH (Mean Squared Error Histogram), NMIH (Normalized Mutual Information Histogram)" to the end of the description for costMetric in BRAINSFitPrimary.xml then add KS, MRSD, MIH, GD, CCH, MSEH, and NMIH as enumeration elements.
set(RUN_EXTRA_METRIC_TESTS OFF)
if(RUN_EXTRA_METRIC_TESTS)
  # KullbackLeiblerCompareHistogramImageToImageMetric requires training images / infrastructure
  # which is different from the other metrics, making it much harder to incorporate into the
  # BRAINSFIT framework.  This test and the relevant code has been commented out for now.
  #set(BRAINSFitTestName BRAINSFitTest_KLMetricBrainToItself)
  #add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
  #  --compare ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
  #            ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  #  --compareIntensityTolerance 9
  #  --compareRadiusTolerance 0
  #  --compareNumberOfPixelsTolerance 1000
  #  BRAINSFitTest
  #  --costMetric KL
  #  --failureExitCode -1 --writeTransformOnFailure
  #  --numberOfIterations 25
  #  --numberOfHistogramBins 200
  #  --numberOfSamples 72000
  #  --translationScale 250
  #  --minimumStepLength 0.001
  #  --outputVolumePixelType uchar
  #  --transformType ScaleVersor3D
  #  --fixedVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
  #  --movingVolume ${BRAINSCommonLib_DATA_DIRS}/test.nii.gz
  #  --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
  #  --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  #)

  # The Kappa Statistic metric is meant for binary images which is why
  # the fixed and moving images are binarized before the tranfsorm is calculated
  # then the transform is applied to the input image.  This produces somethign
  # close for gray scale images but in the next test using binary images it fails
  # to converge even when given many iterations (which it shouldn't need, since
  # it's registering the same image to itself).
  set(BRAINSFitTestName BRAINSFitTest_KSMetricBrainToItself)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{test.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --costMetric KS
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 25
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType Rigid --initializeTransformMode useMomentsAlign
    --fixedVolume MIDAS{test.nii.gz.md5}
    --movingVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  set(BRAINSFitTestName BRAINSFitTest_KSMetricBrainMaskToItself)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{testMask.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.testMask.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --costMetric KS
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 25
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType Rigid
    #--initializeTransformMode useMomentsAlign
    --fixedVolume MIDAS{testMask.nii.gz.md5}
    --movingVolume MIDAS{testMask.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.testMask.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )


  set(BRAINSFitTestName BRAINSFitTest_KSAffineRotationMasks)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 7
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 777
    BRAINSFitTest
    --costMetric KS
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 131072
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType Affine
    --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
    --maskProcessingMode ROI
    --fixedVolume MIDAS{test.nii.gz.md5}
    --fixedBinaryVolume MIDAS{test.nii.gz.md5}
    --movingVolume MIDAS{rotation.test.nii.gz.md5}
    --movingBinaryVolume MIDAS{rotation.test.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
    --debugLevel 50
  )

  set(BRAINSFitTestName BRAINSFitTest_KSScaleSkewVersorRotationMasks)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{${BRAINSFitTestName}.result.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 11
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --costMetric KS
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 2500
    --numberOfHistogramBins 200
    --numberOfSamples 131072
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType ScaleSkewVersor3D
    --initialTransform MIDAS{BRAINSFitTest_Initializer_RigidRotationNoMasks.mat.md5}
    --maskProcessingMode ROI
    --fixedVolume MIDAS{test.nii.gz.md5}
    --fixedBinaryVolume MIDAS{test.nii.gz.md5}
    --movingVolume MIDAS{rotation.test.nii.gz.md5}
    --movingBinaryVolume MIDAS{rotation.test.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  # Fails to converge when registering a brain to itself.
  set(BRAINSFitTestName BRAINSFitTest_MRSDMetricBrainToItself)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{test.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --costMetric MRSD
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 25
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType Rigid
    --fixedVolume MIDAS{test.nii.gz.md5}
    --movingVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  # Segfault when running this test
  set(BRAINSFitTestName BRAINSFitTest_MIHMetricBrainToItself)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{test.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --costMetric MIH
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 25
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType Rigid --initializeTransformMode useMomentsAlign
    --fixedVolume MIDAS{test.nii.gz.md5}
    --movingVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  # Fails to converge when registering a brain to itself.
  set(BRAINSFitTestName BRAINSFitTest_GDMetricBrainToItself)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{test.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --costMetric GD
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 25
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType Rigid --initializeTransformMode useMomentsAlign
    --fixedVolume MIDAS{test.nii.gz.md5}
    --movingVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  # Segfault when running this test
  set(BRAINSFitTestName BRAINSFitTest_CCHMetricBrainToItself)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{test.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --costMetric CCH
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 25
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType Rigid --initializeTransformMode useMomentsAlign
    --fixedVolume MIDAS{test.nii.gz.md5}
    --movingVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  # Segfault when running this test
  set(BRAINSFitTestName BRAINSFitTest_MSEHMetricBrainToItself)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{test.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --costMetric MSEH
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 25
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType Rigid --initializeTransformMode useMomentsAlign
    --fixedVolume MIDAS{test.nii.gz.md5}
    --movingVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )

  # Segfault when running this test
  set(BRAINSFitTestName BRAINSFitTest_NMIHMetricBrainToItself)
  midas_add_test(NAME ${BRAINSFitTestName} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:BRAINSFitTest>
    --compare MIDAS{test.nii.gz.md5}
              ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --compareIntensityTolerance 9
    --compareRadiusTolerance 0
    --compareNumberOfPixelsTolerance 1000
    BRAINSFitTest
    --costMetric NMIH
    --failureExitCode -1 --writeTransformOnFailure
    --numberOfIterations 25
    --numberOfHistogramBins 200
    --numberOfSamples 72000
    --translationScale 250
    --minimumStepLength 0.001
    --outputVolumePixelType uchar
    --transformType Rigid --initializeTransformMode useMomentsAlign
    --fixedVolume MIDAS{test.nii.gz.md5}
    --movingVolume MIDAS{test.nii.gz.md5}
    --outputVolume ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.test.nii.gz
    --outputTransform ${BRAINSFit_BINARY_DIR}/Testing/${BRAINSFitTestName}.mat
  )
endif(RUN_EXTRA_METRIC_TESTS)


