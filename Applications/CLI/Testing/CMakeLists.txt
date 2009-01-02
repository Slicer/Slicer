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
add_test(CurvatureAnisotropicDiffusionTest ${Slicer3_EXE} --launch CurvatureAnisotropicDiffusionTest
  --compare ${BASELINE}/CurvatureAnisotropicDiffusionTest.nhdr
            ${TEMP}/CurvatureAnisotropicDiffusionTest.nhdr
  CurvatureAnisotropicDiffusionTest
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/CurvatureAnisotropicDiffusionTest.nhdr
  )

set (CLP CurvatureAnisotropicDiffusion)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# DicomToNrrdConverter tests
add_test(DicomToNrrdConverterTest ${Slicer3_EXE} --launch DicomToNrrdConverterTest
  DicomToNrrdConverterTest --help
  )

set (CLP DicomToNrrdConverter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# DiffusionTensorEstimation tests
add_test(DiffusionTensorEstimationTest ${Slicer3_EXE} --launch DiffusionTensorEstimationTest
  DiffusionTensorEstimationTest --help
  )

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
  DiffusionTensorMathematicsTest --help
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
  ExecutionModelTourTest --help
  )

set (CLP ExecutionModelTour)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters MRML)

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
  GradientAnisotropicDiffusionTest --help
  )

set (CLP GradientAnisotropicDiffusion)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# GrayscaleFillHoleImageFilter tests
add_test(GrayscaleFillHoleImageFilterTest ${Slicer3_EXE} --launch GrayscaleFillHoleImageFilterTest
  GrayscaleFillHoleImageFilterTest --help
  )

set (CLP GrayscaleFillHoleImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# GrayscaleGrindPeakImageFilter tests
add_test(GrayscaleGrindPeakImageFilterTest ${Slicer3_EXE} --launch GrayscaleGrindPeakImageFilterTest
  GrayscaleGrindPeakImageFilterTest --help
  )

set (CLP GrayscaleGrindPeakImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)

##################################
# GrayscaleModelMaker tests
add_test(GrayscaleModelMakerTest ${Slicer3_EXE} --launch GrayscaleModelMakerTest
  GrayscaleModelMakerTest --help
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
  HistogramMatchingTest --help
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
  ImageReadDicomWriteTest --help
  )

set (CLP ImageReadDicomWrite)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)

##################################
# LabelMapSmoothing tests
add_test(LabelMapSmoothingTest ${Slicer3_EXE} --launch LabelMapSmoothingTest
  LabelMapSmoothingTest --help
  )

set (CLP LabelMapSmoothing)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKAlgorithms ITKBasicFilters)

##################################
# ModelMaker tests
add_test(ModelMakerTest ${Slicer3_EXE} --launch ModelMakerTest
  ModelMakerTest --help
  )

set (CLP ModelMaker)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkITK vtkImaging vtkGraphics vtkIO
  ITKIO
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
  ITKIO
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )

##################################
# OtsuThresholdImageFilter tests
add_test(OtsuThresholdImageFilterTest ${Slicer3_EXE} --launch OtsuThresholdImageFilterTest
  OtsuThresholdImageFilterTest --help
  )

set (CLP OtsuThresholdImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)

##################################
# OtsuThresholdSegmentation tests
add_test(OtsuThresholdSegmentationTest ${Slicer3_EXE} --launch OtsuThresholdSegmentationTest
  OtsuThresholdSegmentationTest --help
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
  ResampleVolume2Test --help
  )

set (CLP ResampleVolume2)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test  ITKIO ITKBasicFilters)

##################################
# ResampleVolume tests
add_test(ResampleVolumeTest ${Slicer3_EXE} --launch ResampleVolumeTest
  ResampleVolumeTest --help
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
  SubtractTest --help
  )

set (CLP Subtract)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test  ITKIO ITKBasicFilters)

##################################
# VotingBinaryHoleFillingImageFilter tests
add_test(VotingBinaryHoleFillingImageFilterTest ${Slicer3_EXE} --launch VotingBinaryHoleFillingImageFilterTest
  VotingBinaryHoleFillingImageFilterTest --help
  )

set (CLP VotingBinaryHoleFillingImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)

##################################
# ZeroCrossingBasedEdgeDetectionImageFilter tests
add_test(ZeroCrossingBasedEdgeDetectionImageFilterTest ${Slicer3_EXE} --launch ZeroCrossingBasedEdgeDetectionImageFilterTest
  ZeroCrossingBasedEdgeDetectionImageFilterTest --help
  )

set (CLP ZeroCrossingBasedEdgeDetectionImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO ITKBasicFilters)
