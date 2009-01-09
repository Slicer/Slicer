#
# Test CLI executables
#
include_regular_expression("^.*$")

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

include_directories(${Slicer3_SOURCE_DIR}/Applications/CLI)
include_directories(${Slicer3_BINARY_DIR}/Applications/CLI)
include_directories(${ModuleDescriptionParser_SOURCE_DIR})
include_directories (${TCLAP_SOURCE_DIR}/include)

#-----------------------------------------------------------------------------
# Configure the default SLICER_BRAINWEB_DATA_ROOT for the location of BrainWeb Data.
# When this data is available, additional 3D tests are enabled.
find_path(SLICER_BRAINWEB_DATA_ROOT brainweb165a10f17.mha 
  DOC "Directory with data taken from http://public.kitware.com/pub/itk/Data/BrainWeb/")
mark_as_advanced(SLICER_BRAINWEB_DATA_ROOT)


#
# Slicer3 is needed to resolve shared libraries
#
set(Slicer3_EXE ${Slicer3_BINARY_DIR}/Slicer3 )

set(BASELINE "${Slicer3_SOURCE_DIR}/Testing/Data/Baseline/CLI")
set(TEST_DATA "${Slicer3_SOURCE_DIR}/Testing/Data/Input")
set(MRML_TEST_DATA "${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/TestData")
set(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")

set(MRML_DATA "${Slicer3_HOME}/share/MRML/Testing/TestData")

# OrientImage tests
add_test(OrientImageTest1 ${Slicer3_EXE} --launch OrientImageTest
  --compare ${BASELINE}/OrientImageTestAxial.nrrd
  ${TEMP}/OrientImageTestAxial.nrrd
  OrientImageTest
  --orientation Axial ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestAxial.nrrd
  )
add_test(OrientImageTest2 ${Slicer3_EXE} --launch OrientImageTest
  --compare ${BASELINE}/OrientImageTestCoronal.nrrd
  ${TEMP}/OrientImageTestCoronal.nrrd
  OrientImageTest
  --orientation Coronal ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestCoronal.nrrd
  )
add_test(OrientImageTest3 ${Slicer3_EXE} --launch OrientImageTest
  --compare ${BASELINE}/OrientImageTestSagittal.nrrd
  ${TEMP}/OrientImageTestSagittal.nrrd
  OrientImageTest
  --orientation Sagittal ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestSagittal.nrrd
  )

set (CLP OrientImage)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

# Add tests
add_test(AddTest ${Slicer3_EXE} --launch AddTest
  --compare ${BASELINE}/AddTest.nhdr
  ${TEMP}/AddTest.nhdr
  AddTest
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/AddTest.nhdr
  )

set (CLP Add)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

# Threshold tests
add_test(ThresholdTest ${Slicer3_EXE} --launch ThresholdTest
  --compare ${BASELINE}/ThresholdTest.nhdr
  ${TEMP}/ThresholdTest.nhdr
  ThresholdTest --upper 3800 --lower 1400 --outsidevalue 0 --thresholdtype Outside
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/ThresholdTest.nhdr
  )

set (CLP Threshold)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

# CheckerBoard tests
add_test(CheckerBoardTest ${Slicer3_EXE} --launch CheckerBoardTest
  --compare ${BASELINE}/CheckerBoardTest.nhdr
  ${TEMP}/CheckerBoardTest.nhdr
  CheckerBoardTest --checkerPattern 5,3,2
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxialDoubled.nhdr ${TEMP}/CheckerBoardTest.nhdr
  )

set (CLP CheckerBoard)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
target_link_libraries(${CLP}Test ITKIO)

# Median tests
add_test(MedianImageFilterTest ${Slicer3_EXE} --launch MedianImageFilterTest
  --compare ${BASELINE}/MedianImageFilterTest.nhdr
  ${TEMP}/MedianImageFilterTest.nhdr
  MedianImageFilterTest
  --neighborhood 1,2,3 ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/MedianImageFilterTest.nhdr
  )

set (CLP MedianImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

# GaussianBlur tests
add_test(GaussianBlurImageFilterTest ${Slicer3_EXE} --launch GaussianBlurImageFilterTest
  --compare ${BASELINE}/GaussianBlurImageFilterTest.nhdr
  ${TEMP}/GaussianBlurImageFilterTest.nhdr
  GaussianBlurImageFilterTest
  --sigma 2.5 ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/GaussianBlurImageFilterTest.nhdr
  )
set (CLP GaussianBlurImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

#
# LinearRegistration tests
#
add_test(LinearRegistrationTest00 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest00.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest00Transform.txt 
  --initialtransform ${TEST_DATA}/itkAffineTransform00.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(LinearRegistrationTest01 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest01.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest01Transform.txt 
  --spatialsamples 10000
  --histogrambins 100
  --iterations 50
  --learningrate 0.001
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(LinearRegistrationTest02 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest02.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest02Transform.txt 
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(LinearRegistrationTest03 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest03.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest03Transform.txt 
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(LinearRegistrationTest04 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest04.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest04Transform.txt 
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
set (CLP LinearRegistration)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKNumerics)

if(SLICER_BRAINWEB_DATA_ROOT)
add_test(LinearRegistrationTest05 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest05.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest05Transform.txt 
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17Rot10Tx15.mha
  )

add_test(LinearRegistrationTest06 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest06.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest06Transform.txt 
  --spatialsamples 50000
  --histogrambins 50
  --iterations 100
  --learningrate 0.01
  --translationscale 100
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17Rot10Tx15.mha
  )

add_test(LinearRegistrationTest07 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest07.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest07Transform.txt 
  --initialtransform ${TEST_DATA}/itkAffineTransform01.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  )

add_test(LinearRegistrationTest08 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest08.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest08Transform.txt 
  --initialtransform ${TEST_DATA}/itkAffineTransform02.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  )

add_test(LinearRegistrationTest09 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest09.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest09Transform.txt 
  --initialtransform ${TEST_DATA}/itkAffineTransform03.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  )

endif(SLICER_BRAINWEB_DATA_ROOT)


#
# BSplineDeformableRegistration tests
#
add_test(BSplineRegistrationTest00 ${Slicer3_EXE} --launch BSplineDeformableRegistrationTest
  BSplineDeformableRegistrationTest
  --resampledmovingfilename  ${TEMP}/BSplineDeformableRegistrationTest00.nhdr
  --outputtransform ${TEMP}/BSplineDeformableRegistrationTest00Transform.txt 
  --default 0
  --maximumDeformation 1.0
  --constrain
  --spatialsamples 10000
  --histogrambins 32
  --gridSize 5
  --iterations 50
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
set (CLP BSplineDeformableRegistration)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKNumerics)

if(SLICER_BRAINWEB_DATA_ROOT)

add_executable(BSplineWarping3DTest BSplineWarping3DTest.cxx)
target_link_libraries(BSplineWarping3DTest ITKIO ITKNumerics)

add_test(BSplineWarping3DTest00 
  ${Slicer3_BINARY_DIR}/bin/BSplineWarping3DTest
  ${TEST_DATA}/BSplineDisplacements3D_00.txt
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb1e1a10f20.mha
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb1e1a10f20.mha
  ${TEMP}/BSplineWarping3DTest00.mhd
)

add_test(BSplineRegistrationTest01 ${Slicer3_EXE} --launch BSplineDeformableRegistrationTest
  BSplineDeformableRegistrationTest
  --resampledmovingfilename  ${TEMP}/BSplineDeformableRegistrationTest01.nhdr
  --outputtransform ${TEMP}/BSplineDeformableRegistrationTest01Transform.txt 
  --default 0
  --maximumDeformation 1.0
  --constrain
  --spatialsamples 10000
  --histogrambins 32
  --gridSize 5
  --iterations 50
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${TEMP}/BSplineWarping3DTest00.mhd
  )

add_test(BSplineRegistrationTest02 ${Slicer3_EXE} --launch BSplineDeformableRegistrationTest
  BSplineDeformableRegistrationTest
  --resampledmovingfilename  ${TEMP}/BSplineDeformableRegistrationTest02.nhdr
  --outputtransform ${TEMP}/BSplineDeformableRegistrationTest02Transform.txt 
  --default 0
  --maximumDeformation 1.0
  --constrain
  --spatialsamples 10000
  --histogrambins 32
  --gridSize 20
  --iterations 500
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${TEMP}/BSplineWarping3DTest00.mhd
  )

add_test(BSplineRegistrationTest03 ${Slicer3_EXE} --launch BSplineDeformableRegistrationTest
  BSplineDeformableRegistrationTest
  --resampledmovingfilename  ${TEMP}/BSplineDeformableRegistrationTest03.nhdr
  --outputtransform ${TEMP}/BSplineDeformableRegistrationTest03Transform.txt 
  --default 0
  --maximumDeformation 5.0
  --constrain
  --spatialsamples 20000
  --histogrambins 32
  --gridSize 5
  --iterations 500
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${TEMP}/BSplineWarping3DTest00.mhd
  )

add_test(BSplineRegistrationTest04 ${Slicer3_EXE} --launch BSplineDeformableRegistrationTest
  BSplineDeformableRegistrationTest
  --resampledmovingfilename  ${TEMP}/BSplineDeformableRegistrationTest04.nhdr
  --outputtransform ${TEMP}/BSplineDeformableRegistrationTest04Transform.txt 
  --default 0
  --maximumDeformation 5.0
  --constrain
  --spatialsamples 40000
  --histogrambins 32
  --gridSize 5
  --iterations 500
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb165a10f17.mha
  ${TEMP}/BSplineWarping3DTest00.mhd
  )


endif(SLICER_BRAINWEB_DATA_ROOT)

# TestGridTransformRegistration tests
add_test(TestGridTransformRegistrationTest00 ${Slicer3_EXE} --launch TestGridTransformRegistrationTest
  TestGridTransformRegistrationTest
  --gridSize 5 
  --outputtransform ${TEMP}/TestGridTransformRegistrationTest00Transform.txt 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
set (CLP TestGridTransformRegistration)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# AffineRegistration tests
add_test(AffineRegistrationTest ${Slicer3_EXE} --launch AffineRegistrationTest
  AffineRegistrationTest  --help
  )

set (CLP AffineRegistration)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKNumerics ITKIO)

##################################
# ConfidenceConnected tests
add_test(ConfidenceConnectedTest ${Slicer3_EXE} --launch ConfidenceConnectedTest
  --compare ${BASELINE}/ConfidenceConnectedTest.nhdr
            ${TEMP}/ConfidenceConnectedTest.nhdr
  ConfidenceConnectedTest
    --smoothingIterations 1
    --timestep 0.0625
    --iterations 5
    --multiplier 2.5
    --neighborhood 1
    --labelvalue 2
    --seed 9.71622,-8.1,14.2189 --seed -12.9459,-8.1,18.577
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ConfidenceConnectedTest.nhdr
  )

set (CLP ConfidenceConnected)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# CurvatureAnisotropicDiffusion tests
set (CLP CurvatureAnisotropicDiffusion)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

add_test(${CLP}Test ${Slicer3_EXE} --launch ${CLP}Test
  --compare ${BASELINE}/${CLP}Test.nhdr
            ${TEMP}/${CLP}Test.nhdr
  ${CLP}Test
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/${CLP}Test.nhdr
  )

##################################
# DicomToNrrdConverter tests
add_test(DicomToNrrdConverterTest ${Slicer3_EXE} --launch DicomToNrrdConverterTest
  DicomToNrrdConverterTest
    ${TEST_DATA}/CTHeadAxialDicom
    ${TEMP} CTHeadAxialFromDicom.nhdr
  )

set (CLP DicomToNrrdConverter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# DiffusionTensorEstimation tests
add_test(DiffusionTensorEstimationTest ${Slicer3_EXE} --launch DiffusionTensorEstimationTest
  DiffusionTensorEstimationTest
    --otsuomegathreshold 0.5
    ${MRML_TEST_DATA}/helix-DWI.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_dti.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_baseline.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_otsu.nhdr  )

set (CLP DiffusionTensorEstimation)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries (${CLP}Test ITKBasicFilters MRML vtkTeem SlicerBaseLogic)
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

##################################
# DiffusionTensorMathematics tests
add_test(DiffusionTensorMathematicsTest ${Slicer3_EXE} --launch DiffusionTensorMathematicsTest
  DiffusionTensorMathematicsTest
    --enumeration FractionalAnisotropy
    ${MRML_TEST_DATA}/helix-DWI.nhdr
    ${TEMP}/DiffusionTensorMathematicsTest.nhdr
  )

set (CLP DiffusionTensorMathematics)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries (${CLP}Test ITKBasicFilters MRML vtkTeem SlicerBaseLogic)

##################################
# DiffusionTensorTest tests
#add_test(DiffusionTensorTest ${Slicer3_EXE} --launch DiffusionTensorTest
#DiffusionTensorTest --help
#)
#
#set (CLP DiffusionTensorTest)
#add_executable(${CLP}Test ${CLP}Test.cxx)
#add_dependencies(${CLP}Test ${CLP})
#target_link_libraries(${CLP}Test ITKIO)
#
##################################
# DiffusionWeightedTest tests
add_test(DiffusionWeightedTestTest ${Slicer3_EXE} --launch DiffusionWeightedTestTest
  DiffusionWeightedTestTest --help
  )

set (CLP DiffusionWeightedTest)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# DWIDicomLoad tests
add_test(DWIDicomLoadTest ${Slicer3_EXE} --launch DWIDicomLoadTest
  DWIDicomLoadTest --help
  )

set (CLP DWIDicomLoad)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# ExecutionModelTour tests
add_test(ExecutionModelTourTest ${Slicer3_EXE} --launch ExecutionModelTourTest
  ExecutionModelTourTest
    --integer 30
    --double 30
    -f 1.3,2,-14
    --string_vector foo,bar,foobar
    --enumeration Bill
    --boolean1
    --transform1 ${TEST_DATA}/ExecutionModelTourTest.mrml\#vtkMRMLLinearTransformNode1
    --transform2 ${TEST_DATA}/ExecutionModelTourTest.mrml\#vtkMRMLLinearTransformNode2
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEST_DATA}/CTHeadAxial.nhdr
  )

set (CLP ExecutionModelTour)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKBasicFilters MRML)

##################################
# FreesurferSurfaceSectionExtraction tests
add_test(FreesurferSurfaceSectionExtractionTest ${Slicer3_EXE} --launch FreesurferSurfaceSectionExtractionTest
  FreesurferSurfaceSectionExtractionTest --help
  )

set (CLP FreesurferSurfaceSectionExtraction)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# GradientAnisotropicDiffusion tests
add_test(GradientAnisotropicDiffusionTest ${Slicer3_EXE} --launch GradientAnisotropicDiffusionTest
  --compare ${BASELINE}/GradientAnisotropicDiffusionTest.nhdr
            ${TEMP}/GradientAnisotropicDiffusionTest.nhdr
  GradientAnisotropicDiffusionTest
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GradientAnisotropicDiffusionTest.nhdr
  )

set (CLP GradientAnisotropicDiffusion)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# GrayscaleFillHoleImageFilter tests
add_test(GrayscaleFillHoleImageFilterTest ${Slicer3_EXE} --launch GrayscaleFillHoleImageFilterTest
  --compare ${BASELINE}/GrayscaleFillHoleTest.nhdr
            ${TEMP}/GrayscaleFillHoleTest.nhdr
  GrayscaleFillHoleImageFilterTest
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GrayscaleFillHoleTest.nhdr
  )

set (CLP GrayscaleFillHoleImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# GrayscaleGrindPeakImageFilter tests
add_test(GrayscaleGrindPeakImageFilterTest ${Slicer3_EXE} --launch GrayscaleGrindPeakImageFilterTest
  --compare ${BASELINE}/GrayscaleGrindPeakTest.nhdr
            ${TEMP}/GrayscaleGrindPeakTest.nhdr
  GrayscaleGrindPeakImageFilterTest
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GrayscaleGrindPeakTest.nhdr
  )

set (CLP GrayscaleGrindPeakImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# GrayscaleModelMaker tests
add_test(GrayscaleModelMakerTest ${Slicer3_EXE} --launch GrayscaleModelMakerTest
  GrayscaleModelMakerTest
    --threshold 300
    --name CTFace
    --smooth 15
    --decimate 0.95
    --splitnormals
    --pointnormals
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEMP}GrayscaleModelMakerTest.vtp
  )

set (CLP GrayscaleModelMaker)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkITK vtkImaging vtkGraphics vtkIO
  ITKIO
  ModuleDescriptionParser
  SlicerBaseCLI
)

##################################
# HistogramMatching tests
add_test(HistogramMatchingTest ${Slicer3_EXE} --launch HistogramMatchingTest
  --compare ${BASELINE}/HistogramMatchingTest.nhdr
            ${TEMP}/HistogramMatchingTest.nhdr
  HistogramMatchingTest
    --numberOfHistogramLevels 64
    --numberOfMatchPoints 10
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/HistogramMatchingTest.nhdr
  )

set (CLP HistogramMatching)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters ITKStatistics)

##################################
# ImageLabelCombine tests
add_test(ImageLabelCombineTest ${Slicer3_EXE} --launch ImageLabelCombineTest
  ImageLabelCombineTest --help
  )

set (CLP ImageLabelCombine)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test  vtkITK vtkImaging vtkGraphics vtkIO vtkTeem)

##################################
# ImageReadDicomWrite tests
add_test(ImageReadDicomWriteTest ${Slicer3_EXE} --launch ImageReadDicomWriteTest
  --compare ${BASELINE}/ImageReadDicomWriteTest.dcm
            ${TEMP}/CTHeadAxialDicom40.dcm
  ImageReadDicomWriteTest
    --patientName Austrialian
    --patientID 8775070
    --patientComments "A volunteer"
    --studyID 123456
    --studyDate 20090102
    --studyComments Resampled
    --studyDescription None
    --modality CT
    --manufacturer "GE Medical Systems"
    --model CT8800
    --seriesNumber 123456
    --seriesDescription None
    --dicomDirectory ${TEMP}
    --dicomPrefix CTHeadAxialDicom
    ${TEST_DATA}/CTHeadAxial.nhdr
  )

set (CLP ImageReadDicomWrite)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)

##################################
# LabelMapSmoothing tests
add_test(LabelMapSmoothingTest ${Slicer3_EXE} --launch LabelMapSmoothingTest
  --compare ${BASELINE}/LabelMapSmoothingTest.nhdr
            ${TEMP}/LabelMapSmoothingTest.nhdr
  LabelMapSmoothingTest
    --labelToSmooth 1
    --numberOfIterations 50
    --maxRMSError 0.01
    --gaussianSigma 3
   ${TEST_DATA}/CTHeadResampledOtsuSegmented.nhdr
   ${TEMP}/LabelMapSmoothingTest.nhdr
  )

set (CLP LabelMapSmoothing)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKAlgorithms ITKBasicFilters)

##################################
# ModelMaker tests

CONFIGURE_FILE(${TEST_DATA}/ModelMakerTest.mrml
    ${TEMP}/ModelMakerTest.mrml IMMEDIATE) 
add_test(ModelMakerTest ${Slicer3_EXE} --launch ModelMakerTest
  ModelMakerTest
    --labels 2
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helix-roi-lable2.nrrd
  )

set (CLP ModelMaker)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkITK vtkImaging vtkGraphics vtkIO
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )

##################################
# MultipleModelsExample tests
add_test(MultipleModelsExampleTest ${Slicer3_EXE} --launch MultipleModelsExampleTest
  MultipleModelsExampleTest --help
  )

set (CLP MultipleModelsExample)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkITK vtkImaging vtkGraphics vtkIO
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )

##################################
# OtsuThresholdImageFilter tests
add_test(OtsuThresholdImageFilterTest ${Slicer3_EXE} --launch OtsuThresholdImageFilterTest
  --compare ${BASELINE}/OtsuThresholdImageFilterTest.nhdr 
            ${TEMP}/OtsuThresholdImageFilterTest.nhdr 
  OtsuThresholdImageFilterTest
    --insideValue 0
    --outsideValue 255
    --numberOfBins 128 
   ${TEST_DATA}/CTHeadResampled.nhdr
   ${TEMP}/OtsuThresholdImageFilterTest.nhdr 
  )

set (CLP OtsuThresholdImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)

##################################
# OtsuThresholdSegmentation tests
add_test(OtsuThresholdSegmentationTest ${Slicer3_EXE} --launch OtsuThresholdSegmentationTest
  --compare ${BASELINE}/OtsuThresholdSegmentationTest.nhdr 
            ${TEMP}/OtsuThresholdSegmentationTest.nhdr 
  OtsuThresholdSegmentationTest
    --brightObjects
    --numberOfBins 128
    --faceConnected
    --minimumObjectSize 1000
   ${TEST_DATA}/CTHeadResampled.nhdr
   ${TEMP}/OtsuThresholdSegmentationTest.nhdr 
)

set (CLP OtsuThresholdSegmentation)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)

##################################
# ProbeVolumeWithModel tests
add_test(ProbeVolumeWithModelTest ${Slicer3_EXE} --launch ProbeVolumeWithModelTest
  ProbeVolumeWithModelTest --help
  )

set (CLP ProbeVolumeWithModel)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test  ITKBasicFilters MRML vtkTeem)

##################################
# ResampleVolume2 tests
add_test(ResampleVolume2Test ${Slicer3_EXE} --launch ResampleVolume2Test
  --compare 
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test.nhdr
  ResampleVolume2Test
    --Reference ${TEST_DATA}/MRHeadResampled.nhdr
    --number_of_thread 0
    --transform_matrix 1,0,0,0,1,0,0,0,1,0,0,0
    --transform rt
    --space LPS
    --interpolation linear
    --window_function c
    --spline_order 3
    --spacing 0,0,0
    --size 0,0,0
    --direction_matrix 0,0,0,0,0,0,0,0,0
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test.nhdr
  )

set (CLP ResampleVolume2)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test  ITKIO ITKBasicFilters)

##################################
# ResampleVolume tests
add_test(ResampleVolumeTest ${Slicer3_EXE} --launch ResampleVolumeTest
  --compare ${BASELINE}/ResampleVolumeTest.nhdr
            ${TEMP}/ResampleVolumeTest.nhdr
  ResampleVolumeTest
    --spacing 5,5,5
    --interpolation linear
   ${TEST_DATA}/MRHeadResampled.nhdr 
   ${TEMP}/ResampleVolumeTest.nhdr
  )

set (CLP ResampleVolume)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# RigidRegistration tests
add_test(RigidRegistrationTest ${Slicer3_EXE} --launch RigidRegistrationTest
  RigidRegistrationTest --help
  )

set (CLP RigidRegistration)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test  ITKIO ITKBasicFilters ITKNumerics)

##################################
# Subtract tests
add_test(SubtractTest ${Slicer3_EXE} --launch SubtractTest
  --compare ${BASELINE}/SubtractTest.nhdr
            ${TEMP}/SubtractTest.nhdr
  SubtractTest
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/CTHeadAxialDoubled.nhdr
    ${TEMP}/SubtractTest.nhdr
  )

set (CLP Subtract)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test  ITKIO ITKBasicFilters)

##################################
# VotingBinaryHoleFillingImageFilter tests
add_test(VotingBinaryHoleFillingImageFilterTest ${Slicer3_EXE} --launch VotingBinaryHoleFillingImageFilterTest
  --compare ${BASELINE}/VotingBinaryHoleFillingImageFilterTest.nhdr
            ${TEMP}/VotingBinaryHoleFillingImageFilterTest.nhdr
  VotingBinaryHoleFillingImageFilterTest
    --radius 3,3,3 
    --majorityThreshold 1
    --background 0
    --foreground 1
   ${TEST_DATA}/CTHeadResampledOtsuSegmented.nhdr
   ${TEMP}/VotingBinaryHoleFillingImageFilterTest.nhdr
  )

set (CLP VotingBinaryHoleFillingImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)

##################################
# ZeroCrossingBasedEdgeDetectionImageFilter tests
add_test(ZeroCrossingBasedEdgeDetectionImageFilterTest ${Slicer3_EXE} --launch ZeroCrossingBasedEdgeDetectionImageFilterTest
  --compare ${BASELINE}/ZeroCrossingBasedEdgeDetectionImageFilterTest.nhdr
            ${TEMP}/ZeroCrossingBasedEdgeDetectionImageFilterTest.nhdr
  ZeroCrossingBasedEdgeDetectionImageFilterTest
    ${TEST_DATA}/CTHeadResampled.nhdr
    ${TEMP}/ZeroCrossingBasedEdgeDetectionImageFilterTest.nhdr
  )

set (CLP ZeroCrossingBasedEdgeDetectionImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)
