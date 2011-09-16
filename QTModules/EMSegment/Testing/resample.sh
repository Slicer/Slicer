#!/bin/bash

set -ex

export Slicer3_HOME=/projects/sandbox/Slicer3/trunk/Slicer3-build
#TARGET=${Slicer3_HOME}/../Slicer3/Modules/EMSegment/Testing/TestData/MiscVolumeData/MRIHumanBrain_T1_aligned.nrrd
TARGET=/home/domibel/Slicer3domibel/RemoteIO/MR-head.nrrd


#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

time ${Slicer3_HOME}/Slicer3 --launch ${Slicer3_HOME}/lib/Slicer3/Plugins/BRAINSFit --fixedVolume ${TARGET} --movingVolume ${Slicer3_HOME}/share/Slicer3/Modules/EMSegment/Tasks/MRI-Human-Brain/atlas_t1.nrrd --outputVolume /tmp/outputbrainsfit.nrrd --bsplineTransform /tmp/transform.mat --maxBSplineDisplacement 10.0 --outputVolumePixelType short --backgroundFillValue 0.0 --interpolationMode Linear --maskProcessingMode  ROIAUTO --ROIAutoDilateSize 3.0 --maskInferiorCutOffFromCenter 65.0 --initializeTransformMode useCenterOfHeadAlign --useRigid --useScaleVersor3D --useScaleSkewVersor3D --useAffine --useBSpline --numberOfSamples 100000 --numberOfIterations 1500 --minimumStepLength 0.005 --translationScale 1000.0 --reproportionScale 1.0 --skewScale 1.0 --splineGridSize 28,20,24 --fixedVolumeTimeIndex 0 --movingVolumeTimeIndex 0 --medianFilterSize 0,0,0 --numberOfHistogramBins 50 --numberOfMatchPoints 10 --useCachingOfBSplineWeightsMode ON --useExplicitPDFDerivativesMode AUTO --relaxationFactor 0.5 --maximumStepLength 0.2 --failureExitCode -1 --debugNumberOfThreads -1 --debugLevel 0 --costFunctionConvergenceFactor 1e+9 --projectedGradientTolerance 1e-5 --costMetric MMI

#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

time ${Slicer3_HOME}/Slicer3 --launch ${Slicer3_HOME}/lib/Slicer3/Plugins/BRAINSResample --inputVolume ${Slicer3_HOME}/share/Slicer3/Modules/EMSegment/Tasks/MRI-Human-Brain/atlas_greymatter.nrrd --referenceVolume ${TARGET} --warpTransform /tmp/transform.mat --outputVolume /tmp/output_resample_only.nrrd --defaultValue 8.0 --pixelType short --interpolationMode Linear

#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

time ${Slicer3_HOME}/Slicer3 --launch ${Slicer3_HOME}/lib/Slicer3/Plugins/BSplineToDeformationField --refImage ${TARGET} --tfm /tmp/transform.mat --defImage /tmp/deformationfield.nrrd

time ${Slicer3_HOME}/Slicer3 --launch ${Slicer3_HOME}/lib/Slicer3/Plugins/BRAINSResample --inputVolume ${Slicer3_HOME}/share/Slicer3/Modules/EMSegment/Tasks/MRI-Human-Brain/atlas_greymatter.nrrd --referenceVolume ${TARGET} --deformationVolume /tmp/deformationfield.nrrd --outputVolume /tmp/output_bspline_and_resample.nrrd --defaultValue 8.0 --pixelType short --interpolationMode Linear

#xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

