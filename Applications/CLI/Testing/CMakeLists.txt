#
# Test CLI executables
#
include_regular_expression("^.*$")

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

include_directories(${Slicer_SOURCE_DIR}/Applications/CLI)
include_directories(${Slicer_BINARY_DIR}/Applications/CLI)
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
set(Slicer_EXE ${Slicer_BINARY_DIR}/Slicer3 )

set(ValgrindScript "${Slicer_BINARY_DIR}/Scripts/runValgrindForOneModule.sh")

if(USE_VALGRIND)
  set(Launcher_Command ${ValgrindScript})
  set(TestLocation "${Slicer_BINARY_DIR}/bin/")
else(USE_VALGRIND)
  set(Launcher_Command ${Slicer_EXE} --launch)
  set(TestLocation "")
endif(USE_VALGRIND)

set(BASELINE "${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI")
set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

set(MRML_DATA "${Slicer_HOME}/share/MRML/Testing/TestData")

# OrientImage tests
add_test(OrientImageTest1 ${Launcher_Command} ${TestLocation}OrientImageTest
  --compare ${BASELINE}/OrientImageTestAxial.nrrd
  ${TEMP}/OrientImageTestAxial.nrrd
  ModuleEntryPoint
  --orientation Axial ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestAxial.nrrd
  )
add_test(OrientImageTest2 ${Launcher_Command} ${TestLocation}OrientImageTest
  --compare ${BASELINE}/OrientImageTestCoronal.nrrd
  ${TEMP}/OrientImageTestCoronal.nrrd
  ModuleEntryPoint
  --orientation Coronal ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestCoronal.nrrd
  )
add_test(OrientImageTest3 ${Launcher_Command} ${TestLocation}OrientImageTest
  --compare ${BASELINE}/OrientImageTestSagittal.nrrd
  ${TEMP}/OrientImageTestSagittal.nrrd
  ModuleEntryPoint
  --orientation Sagittal ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestSagittal.nrrd
  )

set (CLP OrientImage)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

# Add tests
add_test(AddTest ${Launcher_Command} ${TestLocation}AddTest
  --compare ${BASELINE}/AddTest.nhdr
  ${TEMP}/AddTest.nhdr
  ModuleEntryPoint
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/AddTest.nhdr
  )
function(add_module_test pixeltype)
add_test(AddTest_${pixeltype} ${Launcher_Command} ${TestLocation}AddTest
  --compare ${BASELINE}/AddTest_${pixeltype}.mha
  ${TEMP}/AddTest_${pixeltype}.mha
  ModuleEntryPoint
  ${TEST_DATA}/brainSlice${pixeltype}.mha ${TEST_DATA}/brainSlice${pixeltype}.mha ${TEMP}/AddTest_${pixeltype}.mha
  )
endfunction()
add_module_test( CHAR )
add_module_test( UCHAR )
add_module_test( SHORT )
add_module_test( USHORT )
add_module_test( INT )
add_module_test( UINT )
add_module_test( LONG )
add_module_test( ULONG )
add_module_test( FLOAT )
add_module_test( DOUBLE )

set (CLP Add)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

# Mask tests
add_test(MaskTest ${Launcher_Command} ${TestLocation}MaskTest
  --compare ${BASELINE}/MaskedVolume.nrrd
  ${TEMP}/MaskedVolume.nrrd
  ModuleEntryPoint --label 1
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxialMask.nrrd ${TEMP}/MaskedVolume.nrrd
  )

set (CLP Mask)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

# Threshold tests
add_test(ThresholdTest ${Launcher_Command} ${TestLocation}ThresholdTest
  --compare ${BASELINE}/ThresholdTest.nhdr
  ${TEMP}/ThresholdTest.nhdr
  ModuleEntryPoint --upper 3800 --lower 1400 --outsidevalue 0 --thresholdtype Outside
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/ThresholdTest.nhdr
  )

set (CLP Threshold)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

# Cast tests
add_test(CastTest ${Launcher_Command} ${TestLocation}CastTest
  --compare ${BASELINE}/CastTest.nrrd
  ${TEMP}/CastTest.nrrd
  ModuleEntryPoint --type UnsignedChar
  ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTest.nrrd
  )
add_test(CastTestChar ${Launcher_Command} ${TestLocation}CastTest
  --compare ${BASELINE}/CastTestChar.nrrd
  ${TEMP}/CastTestChar.nrrd
  ModuleEntryPoint --type Char
  ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTestChar.nrrd
  )
add_test(CastTestShort ${Launcher_Command} ${TestLocation}CastTest
  --compare ${BASELINE}/CastTestShort.nrrd
  ${TEMP}/CastTestShort.nrrd
  ModuleEntryPoint --type Short
  ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTestShort.nrrd
  )
add_test(CastTestUnsignedShort ${Launcher_Command} ${TestLocation}CastTest
  --compare ${BASELINE}/CastTestUnsignedShort.nrrd
  ${TEMP}/CastTestUnsignedShort.nrrd
  ModuleEntryPoint --type UnsignedShort
  ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTestUnsignedShort.nrrd
  )
add_test(CastTestInt ${Launcher_Command} ${TestLocation}CastTest
  --compare ${BASELINE}/CastTestInt.nrrd
  ${TEMP}/CastTestInt.nrrd
  ModuleEntryPoint --type Int
  ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTestInt.nrrd
  )
add_test(CastTestUnsignedInt ${Launcher_Command} ${TestLocation}CastTest
  --compare ${BASELINE}/CastTestUnsignedInt.nrrd
  ${TEMP}/CastTestUnsignedInt.nrrd
  ModuleEntryPoint --type UnsignedInt
  ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTestUnsignedInt.nrrd
  )
add_test(CastTestFloat ${Launcher_Command} ${TestLocation}CastTest
  --compare ${BASELINE}/CastTestFloat.nrrd
  ${TEMP}/CastTestFloat.nrrd
  ModuleEntryPoint --type Float
  ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTestFloat.nrrd
  )
add_test(CastTestDouble ${Launcher_Command} ${TestLocation}CastTest
  --compare ${BASELINE}/CastTestDouble.nrrd
  ${TEMP}/CastTestDouble.nrrd
  ModuleEntryPoint --type Double
  ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTestDouble.nrrd
  )
set (CLP Cast)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)


# CheckerBoard tests
add_test(CheckerBoardTest ${Launcher_Command} ${TestLocation}CheckerBoardTest
  --compare ${BASELINE}/CheckerBoardTest.nhdr
  ${TEMP}/CheckerBoardTest.nhdr
  ModuleEntryPoint --checkerPattern 5,3,2
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxialDoubled.nhdr ${TEMP}/CheckerBoardTest.nhdr
  )

set (CLP CheckerBoard)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

# Median tests
add_test(MedianImageFilterTest ${Launcher_Command} ${TestLocation}MedianImageFilterTest
  --compare ${BASELINE}/MedianImageFilterTest.nhdr
  ${TEMP}/MedianImageFilterTest.nhdr
  ModuleEntryPoint
  --neighborhood 1,2,3 ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/MedianImageFilterTest.nhdr
  )

add_test(MedianImageFilterTest2 ${Launcher_Command} ${TestLocation}MedianImageFilterTest
  ModuleEntryPoint
  --help
  )

set (CLP MedianImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

# GaussianBlur tests
add_test(GaussianBlurImageFilterTest ${Launcher_Command} ${TestLocation}GaussianBlurImageFilterTest
  --compare ${BASELINE}/GaussianBlurImageFilterTest.nhdr
  ${TEMP}/GaussianBlurImageFilterTest.nhdr
  ModuleEntryPoint
  --sigma 2.5 ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/GaussianBlurImageFilterTest.nhdr
  )
set (CLP GaussianBlurImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

#
# LinearRegistration tests
#
add_test(LinearRegistrationTest00 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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
add_test(LinearRegistrationTest01 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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
add_test(LinearRegistrationTest02 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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
add_test(LinearRegistrationTest03 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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
add_test(LinearRegistrationTest04 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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
target_link_libraries(${CLP}Test ${CLP}Lib)

if(SLICER_BRAINWEB_DATA_ROOT)
add_test(LinearRegistrationTest05 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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

add_test(LinearRegistrationTest06 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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

add_test(LinearRegistrationTest07 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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

add_test(LinearRegistrationTest08 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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

add_test(LinearRegistrationTest09 ${Launcher_Command} ${TestLocation}LinearRegistrationTest
  ModuleEntryPoint
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
add_test(BSplineRegistrationTest00 ${Launcher_Command} ${TestLocation}BSplineDeformableRegistrationTest
  ModuleEntryPoint
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
target_link_libraries(${CLP}Test ${CLP}Lib)

if(SLICER_BRAINWEB_DATA_ROOT)

add_executable(BSplineWarping3DTest BSplineWarping3DTest.cxx)
target_link_libraries(BSplineWarping3DTest ITKIO ITKNumerics)

add_test(BSplineWarping3DTest00 
  ${Slicer_BINARY_DIR}/bin/BSplineWarping3DTest
  ${TEST_DATA}/BSplineDisplacements3D_00.txt
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb1e1a10f20.mha
  ${SLICER_BRAINWEB_DATA_ROOT}/brainweb1e1a10f20.mha
  ${TEMP}/BSplineWarping3DTest00.mhd
)

add_test(BSplineRegistrationTest01 ${Launcher_Command} ${TestLocation}BSplineDeformableRegistrationTest
  ModuleEntryPoint
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

add_test(BSplineRegistrationTest02 ${Launcher_Command} ${TestLocation}BSplineDeformableRegistrationTest
  ModuleEntryPoint
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

add_test(BSplineRegistrationTest03 ${Launcher_Command} ${TestLocation}BSplineDeformableRegistrationTest
  ModuleEntryPoint
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

add_test(BSplineRegistrationTest04 ${Launcher_Command} ${TestLocation}BSplineDeformableRegistrationTest
  ModuleEntryPoint
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
add_test(TestGridTransformRegistrationTest00 ${Launcher_Command} ${TestLocation}TestGridTransformRegistrationTest
  ModuleEntryPoint
  --gridSize 5 
  --outputtransform ${TEMP}/TestGridTransformRegistrationTest00Transform.txt 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
set (CLP TestGridTransformRegistration)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# AffineRegistration tests
add_test(AffineRegistrationTest000 ${Launcher_Command} ${TestLocation}AffineRegistrationTest
  ModuleEntryPoint  --help
  )
add_test(AffineRegistrationTest00 ${Launcher_Command} ${TestLocation}AffineRegistrationTest
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/AffineRegistrationTest00.nhdr
  --outputtransform ${TEMP}/AffineRegistrationTest00Transform.txt 
  --initialtransform ${TEST_DATA}/itkAffineTransform00.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
 --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(AffineRegistrationTest01 ${Launcher_Command} ${TestLocation}AffineRegistrationTest
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/AffineRegistrationTest01.nhdr
  --outputtransform ${TEMP}/AffineRegistrationTest01Transform.txt 
  --spatialsamples 10000
  --histogrambins 100
  --iterations 50
 --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(AffineRegistrationTest02 ${Launcher_Command} ${TestLocation}AffineRegistrationTest
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/AffineRegistrationTest02.nhdr
  --outputtransform ${TEMP}/AffineRegistrationTest02Transform.txt 
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
 --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
set (CLP AffineRegistration)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# ConfidenceConnected tests
add_test(ConfidenceConnectedTest ${Launcher_Command} ${TestLocation}ConfidenceConnectedTest
  --compare ${BASELINE}/ConfidenceConnectedTest.nhdr
            ${TEMP}/ConfidenceConnectedTest.nhdr
    ModuleEntryPoint
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
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# CurvatureAnisotropicDiffusion tests
#set (CLP CurvatureAnisotropicDiffusion)
#add_executable(${CLP}Test ${CLP}Test.cxx)
#target_link_libraries(${CLP}Test ${CLP}Lib)

#add_test(${CLP}Test ${Launcher_Command} ${TestLocation}${CLP}Test
#  --compare ${BASELINE}/${CLP}Test.nhdr
#            ${TEMP}/${CLP}Test.nhdr
#    ModuleEntryPoint
#    --conductance 2
#    --timeStep 0.0625
#    --iterations 2
#    ${TEST_DATA}/MRHeadResampled.nhdr
#    ${TEMP}/${CLP}Test.nhdr
#  )

##################################
# DiffusionTensorEstimation tests
add_test(DiffusionTensorEstimationTest ${Launcher_Command} ${TestLocation}DiffusionTensorEstimationTest
  DiffusionTensorEstimationTest
    --otsuomegathreshold 0.5
    ${MRML_TEST_DATA}/helix-DWI.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_dti.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_baseline.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_otsu.nhdr  )

set (CLP DiffusionTensorEstimation)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries (${CLP}Test MRML vtkTeem SlicerBaseLogic)
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

##################################
# DiffusionTensorMathematics tests
add_test(DiffusionTensorMathematicsTest ${Launcher_Command} ${TestLocation}DiffusionTensorMathematicsTest
  DiffusionTensorMathematicsTest
    --enumeration FractionalAnisotropy
    ${MRML_TEST_DATA}/helix-DTI.nhdr
    ${TEMP}/DiffusionTensorMathematicsTest.nhdr
  )

set (CLP DiffusionTensorMathematics)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries (${CLP}Test MRML vtkTeem SlicerBaseLogic)

##################################
# DiffusionTensorTest tests
set (CLP DiffusionTensorTest)
add_test(${CLP}Test ${Launcher_Command} ${TestLocation}${CLP}Test
  ModuleEntryPoint
    ${MRML_TEST_DATA}/helix-DTI.nhdr
    ${TEMP}/DiffusionTensorTestTest.nhdr
)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# DiffusionWeightedTest tests
#add_test(DiffusionWeightedTestTest ${Launcher_Command} ${TestLocation}DiffusionWeightedTestTest
#  ModuleEntryPoint
#    ${MRML_TEST_DATA}/helix-DWI.nhdr
#    ${TEMP}/DiffusionWeightedTestTest.nhdr
#  )

#set (CLP DiffusionWeightedTest)
#add_executable(${CLP}Test ${CLP}Test.cxx)
#target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# DWIDicomLoad tests
# add_test(DWIDicomLoadTest ${Launcher_Command} ${TestLocation}DWIDicomLoadTest
#  DWIDicomLoadTest --help
#  )

# set (CLP DWIDicomLoad)
# add_executable(${CLP}Test ${CLP}Test.cxx)
# add_dependencies(${CLP}Test ${CLP})
# target_link_libraries(${CLP}Test ITKIO)

##################################
# ExecutionModelTour tests
add_test(ExecutionModelTourTest ${Launcher_Command} ${TestLocation}ExecutionModelTourTest
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
target_link_libraries(${CLP}Test MRML)

##################################
# FreesurferSurfaceSectionExtraction tests
#add_test(FreesurferSurfaceSectionExtractionTest ${Launcher_Command} ${TestLocation}FreesurferSurfaceSectionExtractionTest
#FreesurferSurfaceSectionExtractionTest --help
#)

#set (CLP FreesurferSurfaceSectionExtraction)
#add_executable(${CLP}Test ${CLP}Test.cxx)
#add_dependencies(${CLP}Test ${CLP})
#target_link_libraries(${CLP}Test ITKIO)

##################################
# GradientAnisotropicDiffusion tests
add_test(GradientAnisotropicDiffusionTest ${Launcher_Command} ${TestLocation}GradientAnisotropicDiffusionTest
  --compare ${BASELINE}/GradientAnisotropicDiffusionTest.nhdr
            ${TEMP}/GradientAnisotropicDiffusionTest.nhdr
  ModuleEntryPoint
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GradientAnisotropicDiffusionTest.nhdr
  )

set (CLP GradientAnisotropicDiffusion)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# GrayscaleFillHoleImageFilter tests
add_test(GrayscaleFillHoleImageFilterTest ${Launcher_Command} ${TestLocation}GrayscaleFillHoleImageFilterTest
  --compare ${BASELINE}/GrayscaleFillHoleTest.nhdr
            ${TEMP}/GrayscaleFillHoleTest.nhdr
  ModuleEntryPoint
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GrayscaleFillHoleTest.nhdr
  )

set (CLP GrayscaleFillHoleImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# GrayscaleGrindPeakImageFilter tests
add_test(GrayscaleGrindPeakImageFilterTest ${Launcher_Command} ${TestLocation}GrayscaleGrindPeakImageFilterTest
  --compare ${BASELINE}/GrayscaleGrindPeakTest.nhdr
            ${TEMP}/GrayscaleGrindPeakTest.nhdr
  ModuleEntryPoint
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GrayscaleGrindPeakTest.nhdr
  )

set (CLP GrayscaleGrindPeakImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# GrayscaleModelMaker tests
add_test(GrayscaleModelMakerTest ${Launcher_Command} ${TestLocation}GrayscaleModelMakerTest
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
  vtkITK
  ITKIO
  ModuleDescriptionParser
  SlicerBaseCLI
  vtkHybrid
)

##################################
# HistogramMatching tests
add_test(HistogramMatchingTest ${Launcher_Command} ${TestLocation}HistogramMatchingTest
  --compare ${BASELINE}/HistogramMatchingTest.nhdr
            ${TEMP}/HistogramMatchingTest.nhdr
  ModuleEntryPoint
    --numberOfHistogramLevels 64
    --numberOfMatchPoints 10
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/HistogramMatchingTest.nhdr
  )

set (CLP HistogramMatching)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# ImageLabelCombine tests
add_test(ImageLabelCombineTest ${Launcher_Command} ${TestLocation}ImageLabelCombineTest
  ImageLabelCombineTest --help
  )

set (CLP ImageLabelCombine)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test  ITKIO ITKBasicFilters)

##################################
# ImageReadDicomWrite tests
add_test(ImageReadDicomWriteTest ${Launcher_Command} ${TestLocation}ImageReadDicomWriteTest
  --compare ${BASELINE}/ImageReadDicomWriteTest.dcm
            ${TEMP}/CTHeadAxialDicom0040.dcm
  ModuleEntryPoint
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
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# LabelMapSmoothing tests
add_test(LabelMapSmoothingTest ${Launcher_Command} LabelMapSmoothingTest
  --compare ${BASELINE}/LabelMapSmoothingTest.nhdr
            ${TEMP}/LabelMapSmoothingTest.nhdr
  ModuleEntryPoint
    --labelToSmooth 1
    --numberOfIterations 50
    --maxRMSError 0.01
    --gaussianSigma 3
   ${TEST_DATA}/CTHeadResampledOtsuSegmented.nhdr
   ${TEMP}/LabelMapSmoothingTest.nhdr
  )

set (CLP LabelMapSmoothing)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# ModelMaker tests

CONFIGURE_FILE(${TEST_DATA}/ModelMakerTest.mrml
    ${TEMP}/ModelMakerTest.mrml) 
add_test(ModelMakerTest ${Launcher_Command} ${TestLocation}ModelMakerTest
  ModelMakerTest
    --labels 2
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helix-roi-lable2.nrrd
  )
add_test(ModelMakerGenerateAllOneLabelTest ${Launcher_Command} ${TestLocation}ModelMakerTest
  ModelMakerTest
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helix-roi-lable2.nrrd
  )
add_test(ModelMakerGenerateAllThreeLabelsTest ${Launcher_Command} ${TestLocation}ModelMakerTest
  ModelMakerTest
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
add_test(ModelMakerStartEndTest ${Launcher_Command} ${TestLocation}ModelMakerTest
  ModelMakerTest
    --start 1 --end 5
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
add_test(ModelMakerLabelsTest ${Launcher_Command} ${TestLocation}ModelMakerTest
  ModelMakerTest
    --labels 3,1,5
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
set (CLP ModelMaker)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkITK
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )

##################################
# MultipleModelsExample tests
add_test(MultipleModelsExampleTest ${Launcher_Command} ${TestLocation}MultipleModelsExampleTest
  MultipleModelsExampleTest --help
  )

set (CLP MultipleModelsExample)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkITK
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )

##################################
# OtsuThresholdImageFilter tests
add_test(OtsuThresholdImageFilterTest ${Launcher_Command} ${TestLocation}OtsuThresholdImageFilterTest
  --compare ${BASELINE}/OtsuThresholdImageFilterTest.nhdr 
            ${TEMP}/OtsuThresholdImageFilterTest.nhdr 
   ModuleEntryPoint
    --insideValue 0
    --outsideValue 255
    --numberOfBins 128 
   ${TEST_DATA}/CTHeadResampled.nhdr
   ${TEMP}/OtsuThresholdImageFilterTest.nhdr 
  )

set (CLP OtsuThresholdImageFilter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# OtsuThresholdSegmentation tests
add_test(OtsuThresholdSegmentationTest ${Launcher_Command} ${TestLocation}OtsuThresholdSegmentationTest
  --compare ${BASELINE}/OtsuThresholdSegmentationTest.nhdr 
            ${TEMP}/OtsuThresholdSegmentationTest.nhdr 
  ModuleEntryPoint
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
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################

set(TRANSFORMS_DIR ${Slicer_SOURCE_DIR}/Applications/CLI/DiffusionApplications/ResampleDTI/Testing/TestData )
# ResampleVolume2 tests
add_test(ResampleVolume2Test ${Launcher_Command} ${TestLocation}ResampleVolume2Test
  --compare 
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test.nrrd
  ModuleEntryPoint
    --Reference ${TEST_DATA}/MRHeadResampled.nhdr
    --number_of_thread 0
    --transform_matrix 1,0,0,0,1,0,0,0,1,0,0,0
    --transform rt
    --interpolation linear
    --window_function c
    --spline_order 3
    --spacing 0,0,0
    --size 0,0,0
    --direction_matrix 0,0,0,0,0,0,0,0,0
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test.nrrd
  )

set(TransformFile ${TRANSFORMS_DIR}/rotation.tfm)
add_test(ResampleVolume2RotationNNTest ${Launcher_Command} ${TestLocation}ResampleVolume2Test
  --compare
    ${TEST_DATA}/MRHeadResampledRotationNN.nrrd
    ${TEMP}/ResampleVolume2Test2.nrrd
  ModuleEntryPoint
    -f ${TransformFile}
    --interpolation nn
    -c
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test2.nrrd
    -n 8
  )

set(RotationAndAffineFile ${TRANSFORMS_DIR}/rotationAndAffine.tfm)
add_test(ResampleVolume2RotationAndAffineTest ${Launcher_Command} ${TestLocation}ResampleVolume2Test
  --compare
    ${TEST_DATA}/MRHeadResampledRotationAndAffine.nrrd
    ${TEMP}/ResampleVolume2Test3.nrrd
  ModuleEntryPoint
    -f ${RotationAndAffineFile}
    --interpolation linear
    -c
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test3.nrrd
    --transform_order input-to-output
  )

set(BSplineFile ${Slicer_SOURCE_DIR}/Testing/Data/Input/MRHeadResampledBSplineTransform.tfm)
add_test(ResampleVolume2BSplineWSInterpolationTest ${Launcher_Command} ${TestLocation}ResampleVolume2Test
  --compare
    ${TEST_DATA}/MRHeadResampledBSplineWSInterpolationTest.nrrd
    ${TEMP}/ResampleVolume2Test4.nrrd
  ModuleEntryPoint
    -f ${BSplineFile}
    --interpolation ws
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test4.nrrd
    --transform_order input-to-output

  )

set(AffineFile ${TRANSFORMS_DIR}/affine.tfm)
add_test(ResampleVolume2BSplineInterpolationTest ${Launcher_Command} ${TestLocation}ResampleVolume2Test
  --compare
    ${TEST_DATA}/MRHeadResampledBSplineInterpolationTest.nrrd
    ${TEMP}/ResampleVolume2Test5.nrrd
  ModuleEntryPoint
    -f ${AffineFile}
    --interpolation bs
    -o 3
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test5.nrrd
  )


set(HFieldFile ${Slicer_SOURCE_DIR}/Testing/Data/Input/MRHeadResampledHField.nrrd)
add_test(ResampleVolume2HFieldTest ${Launcher_Command} ${TestLocation}ResampleVolume2Test
  --compare
    ${TEST_DATA}/MRHeadResampledHFieldTest.nrrd
    ${TEMP}/ResampleVolume2Test6.nrrd
  ModuleEntryPoint
    -H ${HFieldFile}
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test6.nrrd
  )


set (CLP ResampleVolume2)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# ResampleVolume tests
add_test(ResampleVolumeTest ${Launcher_Command} ${TestLocation}ResampleVolumeTest
  --compare ${BASELINE}/ResampleVolumeTest.nhdr
            ${TEMP}/ResampleVolumeTest.nhdr
  ModuleEntryPoint
    --spacing 5,5,5
    --interpolation linear
   ${TEST_DATA}/MRHeadResampled.nhdr 
   ${TEMP}/ResampleVolumeTest.nhdr
  )

set (CLP ResampleVolume)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# RigidRegistration tests
add_test(RigidRegistrationTest000 ${Launcher_Command} ${TestLocation}RigidRegistrationTest
  ModuleEntryPoint --help
  )
add_test(RigidRegistrationTest00 ${Launcher_Command} ${TestLocation}RigidRegistrationTest
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest00.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest00Transform.txt 
  --initialtransform ${TEST_DATA}/itkAffineTransform00.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(RigidRegistrationTest01 ${Launcher_Command} ${TestLocation}RigidRegistrationTest
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest01.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest01Transform.txt 
  --spatialsamples 10000
  --histogrambins 100
  --iterations 50
  --learningrate 0.001
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(RigidRegistrationTest02 ${Launcher_Command} ${TestLocation}RigidRegistrationTest
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest02.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest02Transform.txt 
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(RigidRegistrationTest03 ${Launcher_Command} ${TestLocation}RigidRegistrationTest
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest03.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest03Transform.txt 
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(RigidRegistrationTest04 ${Launcher_Command} ${TestLocation}RigidRegistrationTest
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest04.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest04Transform.txt 
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )

# This test reproduces exactly the registration step on
# the meningioma growth detection (tutorial) data for ChangeTracker
# Setting parameters explicitly, because default values are not passed to CLI
# In testing mode, this test is supposed to fail if the error at the image
# corner (as compared to the transform passed with --initialtransform) exceeds
# 0.1
#add_test(RigidRegistrationTest02 ${Launcher_Command} ${TestLocation}RigidRegistrationTest
#  --compare ${BASELINE}/RigidRegistrationTest02.nrrd
#   ${TEMP}/RigidRegistrationTest02.nrrd
#  RigidRegistrationTest
#  --testingmode
#  --initialtransform ${TEST_DATA}/MRMeningioma01.tfm
#  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest02.nrrd
#  --outputtransform ${TEMP}/RigidRegistrationTest02Transform.txt
#  --iterations 100,100,50,20
#  --fixedsmoothingfactor 0
#  --movingsmoothingfactor 0
#  --histogrambins 30
#  --spatialsamples 10000
#  --learningrate 0.01,0.005,0.0005,0.0002
#  --translationscale 100
#  ${TEST_DATA}/MRMeningioma0.nrrd
#  ${TEST_DATA}/MRMeningioma1.nrrd
#  )
set (CLP RigidRegistration)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# Subtract tests
add_test(SubtractTest ${Launcher_Command} ${TestLocation}SubtractTest
  --compare ${BASELINE}/SubtractTest.nhdr
            ${TEMP}/SubtractTest.nhdr
  ModuleEntryPoint
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/CTHeadAxialDoubled.nhdr
    ${TEMP}/SubtractTest.nhdr
  )

set (CLP Subtract)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# Multiply tests
add_test(MultiplyTest ${Launcher_Command} ${TestLocation}MultiplyTest
  --compare ${BASELINE}/MultiplyTest.nrrd
            ${TEMP}/MultiplyTest.nrrd
  ModuleEntryPoint
    --order 1
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEMP}/MultiplyTest.nrrd
  )

set (CLP Multiply)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# VotingBinaryHoleFillingImageFilter tests
add_test(VotingBinaryHoleFillingImageFilterTest ${Launcher_Command} ${TestLocation}VotingBinaryHoleFillingImageFilterTest
  --compare ${BASELINE}/VotingBinaryHoleFillingImageFilterTest.nhdr
            ${TEMP}/VotingBinaryHoleFillingImageFilterTest.nhdr
  ModuleEntryPoint
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
target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# culled from Slicer 3.6
# ZeroCrossingBasedEdgeDetectionImageFilter tests
#add_test(ZeroCrossingBasedEdgeDetectionImageFilterTest ${Launcher_Command} ${TestLocation}ZeroCrossingBasedEdgeDetectionImageFilterTest
#--compare ${BASELINE}/ZeroCrossingBasedEdgeDetectionImageFilterTest.nhdr
#${TEMP}/ZeroCrossingBasedEdgeDetectionImageFilterTest.nhdr
#ModuleEntryPoint
#${TEST_DATA}/CTHeadResampled.nhdr
#${TEMP}/ZeroCrossingBasedEdgeDetectionImageFilterTest.nhdr
#)
#
#set (CLP ZeroCrossingBasedEdgeDetectionImageFilter)
#add_executable(${CLP}Test ${CLP}Test.cxx)
#target_link_libraries(${CLP}Test ${CLP}Lib)

##################################
# PolyDataToLabelmap tests
add_test(PolyDataToLabelmapTest ${Launcher_Command} ${TestLocation}PolyDataToLabelmapTest
  --compare ${BASELINE}/OAS10001.mha
            ${TEMP}/PolyDataToLabelmapTestOutput.mha
  ModuleEntryPoint
    ${TEST_DATA}/PolyDataToLabelmap/OAS10001.hdr 
    ${TEST_DATA}/PolyDataToLabelmap/OAS10001.vtp 
    ${TEMP}/PolyDataToLabelmapTestOutput.mha
  )

set (CLP PolyDataToLabelmap)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)


##################################
# SparseFieldLevelSetContour tests

# leaks from tcl file
# add_test(SparseFieldLevelSetContourTest2 ${Slicer_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer_SOURCE_DIR}/Applications/CLI/SparseFieldLevelSetContour/Testing/SparseFieldLevelSetContourTest.tcl  ${TEST_DATA}/SparseFieldLevelSetContourTest.vtp ${TEMP}/SparseFieldLevelSetContourTest.vtp ${BASELINE}/SparseFieldLevelSetContourTest.vtp)

# use a helper to compare the baseline and output, running it from the launcher to set up the paths

# the test code doesn't link on windows
if (NOT WIN32 AND NOT APPLE)
add_test(SparseFieldLevelSetContourTest1 ${Launcher_Command}  ${CMAKE_COMMAND}
  -Dtest_cmd=SparseFieldLevelSetContour1Test
  -Dtest_name=SparseFieldLevelSetContour1Test
  -Dinput_scene=${TEST_DATA}/SparseFieldLevelSetContourTest.vtp
  -Dc1=132.059,-3.68759,-378.683
  -Dc2=127.652,-16.0285,-381.549
  -Dc3=120.15,-27.4267,-382.79
  -Dc4=117.438,-26.4077,-394.14
  -Dc5=128.782,-14.2639,-389.404
  -Dc6=134.297,1.63338,-387.623
  -Doutput_model=${TEMP}/SparseFieldLevelSetContourTest.vtp
  -Doutput_baseline=${BASELINE}/SparseFieldLevelSetContourTest.vtp
  -Doutput_test=${TEMP}/SparseFieldLevelSetContourTest.vtp
  -P ${Slicer_SOURCE_DIR}/Applications/CLI/Testing/run_SparseFieldLevelSetContourTest.cmake
)

set (CLP SparseFieldLevelSetContour1)
include_directories( ${SlicerBaseCLI_BINARY_DIR} 
                     ${CMAKE_CURRENT_BINARY_DIR}/../SparseFieldLevelSetContour  
                     ${CMAKE_CURRENT_SOURCE_DIR}/../SparseFieldLevelSetContour 
                     ${vtkTeem_SOURCE_DIR}  
                     ${vtkTeem_BINARY_DIR} 
                     ) 
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test SparseFieldLevelSetContour)
target_link_libraries(${CLP}Test SparseFieldLevelSetContourLib
  vtkITK  vtkIO SlicerBaseCLI
  ModuleDescriptionParser
  )
endif (NOT WIN32 AND NOT APPLE)

# MergeModels tests

add_test(MergeModelsTest ${Launcher_Command} ${TestLocation}MergeModelsTest
  MergeModelsTest
    ${MRML_TEST_DATA}/sphere.vtk
    ${MRML_TEST_DATA}/cube.vtk
    ${TEMP}/sphereCube.vtk
  )
add_test(MergeModelsTestVtp ${Launcher_Command} ${TestLocation}MergeModelsTest
  MergeModelsTest
    ${MRML_TEST_DATA}/sphere.vtp
    ${MRML_TEST_DATA}/cube.vtp
    ${TEMP}/sphereCube.vtp
  )

# add a test that compares the outline to a baseline .vtp
add_test(MergeModelsTestCompare  ${Launcher_Command} ${CMAKE_COMMAND}
  -Dtest_cmd=MergeModelsTest
  -Dtest_name=MergeModelsTest
  -Dmodel1=${MRML_TEST_DATA}/sphere.vtp
  -Dmodel2=${MRML_TEST_DATA}/cube.vtp
  -Doutput_model=${TEMP}/sphereCube.vtp
  -Doutput_baseline=${BASELINE}/sphereCube.vtp
  -Doutput_test=${TEMP}/sphereCube.vtp
  -P  ${Slicer_SOURCE_DIR}/Applications/CLI/Testing/run_MergeModelsTest.cmake
)

set (CLP MergeModels)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkIO
  ITKIO
  vtkGraphics
  )

# ACPC Realign test


# configure the input mrml file to copy it to the binary TEMP dir so
# the test doesn't modify the source tree one
configure_file(
  ${TEST_DATA}/ACPCTest.mrml
  ${TEMP}/ACPCTest.mrml
  COPYONLY
  )

# now add the test
add_test(ACPCTest ${Launcher_Command} ${TestLocation}ACPCTest
  ModuleEntryPoint
#  --acpc ${TEST_DATA}/ACPC.fcsv --midline ${TEST_DATA}/midsag.fcsv 
  --acpc 0.5,-21.5,-9.5 --acpc -2.5,4.5,-16.5 --midline 12.5,-93.5,7.5 --midline 12.5,-93.5,-12.5 --midline 5.5,-93.5,-25.5 --midline 9.5,-83.5,-22.5 --midline 9.5,-83.5,-12.5 --midline 10.5,-83.5,10.5 --midline 10.5,-83.5,26.5 --midline 7.5,-68.5,38.5 --midline 7.5,-68.5,13.5 --midline 6.5,-68.5,-9.5 --midline 5.5,-68.5,-30.5 --midline 5.5,-68.5,-48.5 --midline 3.5,-49.5,50.5 --midline 4.5,-49.5,25.5 --midline 3.5,-49.5,5.5 --midline 4.5,-49.5,-32.5 --midline 3.5,-34.5,54.5 --midline 1.5,-34.5,30.5 --midline 1.5,-34.5,17.5 --midline 3.5,-34.5,-22.5 --midline 1.5,-19.5,58.5 --midline 0.5,-19.5,34.5 --midline 0.5,-19.5,18.5 --midline 0.5,-19.5,-13.5 --midline 1.5,-19.5,-25.5 --midline -1.5,-4.5,62.5 --midline -2.5,-4.5,36.5 --midline -1.5,-4.5,19.5 --midline -1.5,-4.5,0.5 --midline -1.5,-4.5,-12.5 --midline -1.5,-4.5,-24.5 --midline -1.5,-4.5,-39.5 --midline -3.5,10.5,49.5 --midline -1.5,10.5,28.5 --midline -3.5,10.5,14.5 --midline -2.5,10.5,5.5 --midline -3.5,10.5,-15.5 --midline -1.5,10.5,-33.5 --midline -5.5,26.5,44.5 --midline -6.5,26.5,25.5 --midline -5.5,26.5,3.5 --midline -5.5,26.5,-11.5 --midline -5.5,26.5,-21.5 --midline -5.5,26.5,-35.5 --midline -5.5,41.5,35.5 --midline -7.5,41.5,12.5 --midline -6.5,41.5,-4.5 --midline -7.5,41.5,-27.5 --midline -7.5,41.5,-41.5 --midline -7.5,56.5,24.5 --midline -9.5,56.5,-0.5 --midline -9.5,56.5,-12.5 --midline -9.5,56.5,-29.5 --midline -10.5,71.5,3.5 --midline -10.5,71.5,-12.5 --midline -11.5,71.5,-26.5 --outputTransform ${TEMP}/ACPCTest.mrml\#vtkMRMLLinearTransformNode1
  )

set(CLP ACPC)
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test  RealignLib)

