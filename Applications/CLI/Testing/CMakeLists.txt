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
  --initialtransform ${TEST_DATA}/itkAffineTransform01.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
add_test(LinearRegistrationTest02 ${Slicer3_EXE} --launch LinearRegistrationTest
  LinearRegistrationTest
  --resampledmovingfilename  ${TEMP}/LinearRegistrationTest02.nhdr
  --outputtransform ${TEMP}/LinearRegistrationTest02Transform.txt 
  --initialtransform ${TEST_DATA}/itkAffineTransform02.txt
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
  --initialtransform ${TEST_DATA}/itkAffineTransform03.txt
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
  --initialtransform ${TEST_DATA}/itkAffineTransform04.txt
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
  --gridSize 5 
  --outputtransform ${TEMP}/TestGridTransformRegistrationTest00Transform.txt 
  )
set (CLP TestGridTransformRegistration)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP}Lib)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ITKIO)


