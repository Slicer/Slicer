
SET(CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
SET(TEST_DATA_ROOT ${RegisterImagesModule_SOURCE_DIR})
SET(TEMP ${RegisterImagesModule_BINARY_DIR}/Testing/Temporary)

#-----------------------------------------------------------------------------
# Configure the default ITK_EXAMPLES_DATA_ROOT for the location of ITK Examples/Data.
# When this data is available, additional 2D tests are enabled.
FIND_PATH(ITK_EXAMPLES_DATA_ROOT BrainProtonDensitySlice.png 
  DOC "Directory with data taken from Insight/Examples/Data")

#-----------------------------------------------------------------------------
# Configure the default ITK_BRAINWEB_DATA_ROOT for the location of ITK Data.
# When this data is available, additional 3D tests are enabled.
FIND_PATH(ITK_BRAINWEB_DATA_ROOT brainweb165a10f17.mha 
  DOC "Directory with data taken from http://public.kitware.com/pub/itk/Data/BrainWeb/")

#-----------------------------------------------------------------------------
# Configure the default ITK_MRI_UNC_DATABASE_DATA_ROOT for the location of the
# database of healthy volunteers MRI brain images.
# When this data is available, additional 3D tests are enabled.
FIND_PATH(ITK_MRI_UNC_DATABASE_DATA_ROOT Normal002-T1-Flash.mha
  DOC "Directory with data taken from http://public.kitware.com/pub/itk/Data/HealthyVolunteersMRIDatabase/")

#-----------------------------------------------------------------------------
# Configure the default REGISTRATION_METHODS_BASELINE_DATA_ROOT for the
# location of the database of valid images used as baselines in the regression
# tests.
FIND_PATH(REGISTRATION_METHODS_BASELINE_DATA_ROOT Normal002-Normal004-Affine.mha
  DOC "Directory with valid baseline images")



#
#  Define Executables
#
ADD_EXECUTABLE(itkInitialImageToImageRegistrationMethodTest1 itkInitialImageToImageRegistrationMethodTest1.cxx )
TARGET_LINK_LIBRARIES(itkInitialImageToImageRegistrationMethodTest1 ITKStatistics ITKIO ITKNumerics)

ADD_EXECUTABLE(itkRigidImageToImageRegistrationMethodTest1 itkRigidImageToImageRegistrationMethodTest1.cxx )
TARGET_LINK_LIBRARIES(itkRigidImageToImageRegistrationMethodTest1 ITKStatistics ITKIO ITKNumerics)

ADD_EXECUTABLE(itkAffineImageToImageRegistrationMethodTest1 itkAffineImageToImageRegistrationMethodTest1.cxx )
TARGET_LINK_LIBRARIES(itkAffineImageToImageRegistrationMethodTest1 ITKStatistics ITKIO ITKNumerics)

ADD_EXECUTABLE(itkBSplineImageToImageRegistrationMethodTest1 itkBSplineImageToImageRegistrationMethodTest1.cxx )
TARGET_LINK_LIBRARIES(itkBSplineImageToImageRegistrationMethodTest1 ITKStatistics ITKIO ITKNumerics)

ADD_EXECUTABLE(itkImageToImageRegistrationHelperTest01 itkImageToImageRegistrationHelperTest01.cxx )
TARGET_LINK_LIBRARIES(itkImageToImageRegistrationHelperTest01 ITKStatistics ITKIO ITKNumerics)

ADD_EXECUTABLE(itkImageToImageRegistrationHelperTest02 itkImageToImageRegistrationHelperTest02.cxx )
TARGET_LINK_LIBRARIES(itkImageToImageRegistrationHelperTest02 ITKStatistics ITKIO ITKNumerics)



###################
###################
#
#  Define Tests
#
###################
###################

###################
# Initial
###################
ADD_TEST(InitialRegistrationTestBW2D 
  ${CXX_TEST_PATH}/itkInitialImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/InitialBW2D.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/InitialBW2D.mha
  --DifferenceImage ${TEMP}/InitialBW2D_Diff.mha
  ${ITK_EXAMPLES_DATA_ROOT}/BrainProtonDensitySliceBorder20.png
  ${ITK_EXAMPLES_DATA_ROOT}/BrainProtonDensitySliceR10X13Y17.png
 )

ADD_TEST(InitialRegistrationTestBWR10T15 
  ${CXX_TEST_PATH}/itkInitialImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/InitialBWR10T15.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/InitialBWR10T15.mha
  --DifferenceImage ${TEMP}/InitialBWR10T15_Diff.mha
  ${ITK_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${ITK_BRAINWEB_DATA_ROOT}/brainweb165a10f17Rot10Tx15.mha
  )

ADD_TEST(InitialRegistrationTestUNC24
  ${CXX_TEST_PATH}/itkInitialImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/InitialUNC24.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/InitialUNC24.mha
  --DifferenceImage ${TEMP}/InitialUNC24_Diff.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-T1-Flash.mha
  )

ADD_TEST(InitialRegistrationTestUNC24Stripped
  ${CXX_TEST_PATH}/itkInitialImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/InitialUNC24Stripped.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/InitialUNC24Stripped.mha
  --DifferenceImage ${TEMP}/InitialUNC24Stripped_Diff.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash-Stripped.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-T1-Flash-Stripped.mha
  )

###################
# Rigid
###################
ADD_TEST(RigidRegistrationTestBW2D 
  ${CXX_TEST_PATH}/itkRigidImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/RigidBW2D.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/RigidBW2D.mha
  --DifferenceImage ${TEMP}/RigidBW2D_Diff.mha
  --FailureIntensityTolerance 20
  --FailureOffsetTolerance 1
  ${ITK_EXAMPLES_DATA_ROOT}/BrainProtonDensitySliceBorder20.png
  ${ITK_EXAMPLES_DATA_ROOT}/BrainProtonDensitySliceR10X13Y17.png
 )

ADD_TEST(RigidRegistrationTestBWR10T15 
  ${CXX_TEST_PATH}/itkRigidImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/RigidBWR10T15.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/RigidBWR10T15.mha
  --DifferenceImage ${TEMP}/RigidBWR10T15_Diff.mha
  --FailurePixelTolerance 17000
  --FailureIntensityTolerance 20
  --FailureOffsetTolerance 2
  ${ITK_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${ITK_BRAINWEB_DATA_ROOT}/brainweb165a10f17Rot10Tx15.mha
  )

ADD_TEST(RigidRegistrationTestUNC24
  ${CXX_TEST_PATH}/itkRigidImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/RigidUNC24.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/RigidUNC24.mha
  --DifferenceImage ${TEMP}/RigidUNC24_Diff.mha
  --FailureIntensityTolerance 20
  --FailureOffsetTolerance 1
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-T1-Flash.mha
  )

ADD_TEST(RigidRegistrationTestUNC24Stripped
  ${CXX_TEST_PATH}/itkRigidImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/RigidUNC24Stripped.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/RigidUNC24Stripped.mha
  --DifferenceImage ${TEMP}/RigidUNC24Stripped_Diff.mha
  --FailureIntensityTolerance 20
  --FailureOffsetTolerance 1
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash-Stripped.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-T1-Flash-Stripped.mha
  )

###################
# Affine
###################
ADD_TEST(AffineRegistrationTestBW2D 
  ${CXX_TEST_PATH}/itkAffineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/AffineBW2D.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/AffineBW2D.mha
  --DifferenceImage ${TEMP}/AffineBW2D_Diff.mha
  --FailureIntensityTolerance 20
  --FailureOffsetTolerance 1
  ${ITK_EXAMPLES_DATA_ROOT}/BrainProtonDensitySliceBorder20.png
  ${ITK_EXAMPLES_DATA_ROOT}/BrainProtonDensitySliceR10X13Y17.png
 )

ADD_TEST(AffineRegistrationTestBWR10T15 
  ${CXX_TEST_PATH}/itkAffineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/AffineBWR10T15.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/AffineBWR10T15.mha
  --DifferenceImage ${TEMP}/AffineBWR10T15_Diff.mha
  --FailureIntensityTolerance 40
  --FailureOffsetTolerance 1
  ${ITK_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${ITK_BRAINWEB_DATA_ROOT}/brainweb165a10f17Rot10Tx15.mha
  )

ADD_TEST(AffineRegistrationTestUNC24
  ${CXX_TEST_PATH}/itkAffineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/AffineUNC24.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/AffineUNC24.mha
  --DifferenceImage ${TEMP}/AffineUNC24_Diff.mha
  --FailureIntensityTolerance 40
  --FailureOffsetTolerance 1
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-T1-Flash.mha
  )

ADD_TEST(AffineRegistrationTestUNC24Stripped
  ${CXX_TEST_PATH}/itkAffineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/AffineUNC24Stripped.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/AffineUNC24Stripped.mha
  --DifferenceImage ${TEMP}/AffineUNC24Stripped_Diff.mha
  --FailureIntensityTolerance 40
  --FailureOffsetTolerance 1
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash-Stripped.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-T1-Flash-Stripped.mha
  )

###################
# BSpline
###################
ADD_TEST(BSplineRegistrationTestUNC24
  ${CXX_TEST_PATH}/itkBSplineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/BSplineUNC24.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/BSplineUNC24.mha
  --DifferenceImage ${TEMP}/BSplineUNC24_Diff.mha
  --MinimizeMemory
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-Normal004-Affine.mha
  )

ADD_TEST(BSplineRegistrationTestUNC24Stripped
  ${CXX_TEST_PATH}/itkBSplineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/BSplineUNC24Stripped.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/BSplineUNC24Stripped.mha
  --DifferenceImage ${TEMP}/BSplineUNC24Stripped_Diff.mha
  --MinimizeMemory
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash-Stripped.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-Normal004-Stripped-Affine.mha
  )

ADD_TEST(BSplineRegistrationTestUNC24StrippedC20
  ${CXX_TEST_PATH}/itkBSplineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/BSplineUNC24StrippedC20.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/BSplineUNC24StrippedC20.mha
  --DifferenceImage ${TEMP}/BSplineUNC24StrippedC20_Diff.mha
  --MinimizeMemory
  --NumberOfControlPoints 20
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash-Stripped.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-Normal004-Stripped-Affine.mha
  )

ADD_TEST(BSplineRegistrationTestUNC25Stripped
  ${CXX_TEST_PATH}/itkBSplineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/BSplineUNC25Stripped.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/BSplineUNC25Stripped.mha
  --DifferenceImage ${TEMP}/BSplineUNC25Stripped_Diff.mha
  --MinimizeMemory
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-T1-Flash-Stripped.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal002-Normal005-Stripped-Affine.mha
  )

ADD_TEST(BSplineRegistrationTestUNC42Stripped
  ${CXX_TEST_PATH}/itkBSplineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/BSplineUNC42Stripped.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/BSplineUNC42Stripped.mha
  --DifferenceImage ${TEMP}/BSplineUNC42Stripped_Diff.mha
  --MinimizeMemory
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-T1-Flash-Stripped.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-Normal002-Stripped-Affine.mha
  )

ADD_TEST(BSplineRegistrationTestUNC45Stripped
  ${CXX_TEST_PATH}/itkBSplineImageToImageRegistrationMethodTest1 
  --ResampledMovingImage ${TEMP}/BSplineUNC45Stripped.mha
  --BaselineResampledMovingImage ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/BSplineUNC45Stripped.mha
  --DifferenceImage ${TEMP}/BSplineUNC45Stripped_Diff.mha
  --MinimizeMemory
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-T1-Flash-Stripped.mha
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/Normal004-Normal005-Stripped-Affine.mha
  )

###################
# Helper
###################
ADD_TEST(RegistrationHelperTest01a
  ${CXX_TEST_PATH}/itkImageToImageRegistrationHelperTest01 
  ${ITK_EXAMPLES_DATA_ROOT}/BrainProtonDensitySliceBorder20.png
  ${ITK_EXAMPLES_DATA_ROOT}/BrainProtonDensitySliceR10X13Y17.png
  ${TEMP}/RegistrationHelperTest01a.mha
  ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/RegistrationHelperTest01a.mha
  )

ADD_TEST(RegistrationHelperTest02a
  ${CXX_TEST_PATH}/itkImageToImageRegistrationHelperTest02 
  ${ITK_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${ITK_BRAINWEB_DATA_ROOT}/brainweb165a10f17Rot10Tx15.mha
  ${TEMP}/RegistrationHelperTest02a.mha
  ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/RegistrationHelperTest02a.mha
  )

ADD_TEST(RegistrationHelperTest02b
  ${CXX_TEST_PATH}/itkImageToImageRegistrationHelperTest02 
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/b/I.hdr
  ${ITK_MRI_UNC_DATABASE_DATA_ROOT}/d/I.hdr
  ${TEMP}/RegistrationHelperTest02b.mha
  ${REGISTRATION_METHODS_BASELINE_DATA_ROOT}/RegistrationHelperTest02b.mha
  )

