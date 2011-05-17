#
# Test CLI executables
#
include_regular_expression("^.*$")

find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})


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

set(Launcher_Command ${LAUNCH_EXE})
if(USE_VALGRIND)
  set(Launcher_Command "${Slicer_BINARY_DIR}/Scripts/runValgrindForOneModule.sh")
endif()

set(BASELINE "${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI")
set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
set(MRML_TEST_DATA "${Slicer_SOURCE_DIR}/Libs/MRML/Testing/TestData")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

set(MRML_DATA "${Slicer_HOME}/share/MRML/Testing/TestData")

#-----------------------------------------------------------------------------
# OrientImage tests
#-----------------------------------------------------------------------------
set (CLP OrientImage)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname OrientImageTest1)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OrientImageTestAxial.nrrd
  ${TEMP}/OrientImageTestAxial.nrrd
  ModuleEntryPoint
  --orientation Axial ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestAxial.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname OrientImageTest2)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OrientImageTestCoronal.nrrd
  ${TEMP}/OrientImageTestCoronal.nrrd
  ModuleEntryPoint
  --orientation Coronal ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestCoronal.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname OrientImageTest3)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OrientImageTestSagittal.nrrd
  ${TEMP}/OrientImageTestSagittal.nrrd
  ModuleEntryPoint
  --orientation Sagittal ${MRML_DATA}/fixed.nrrd ${TEMP}/OrientImageTestSagittal.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# Add tests
#-----------------------------------------------------------------------------
set (CLP Add)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/AddTest.nhdr
  ${TEMP}/AddTest.nhdr
  ModuleEntryPoint
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/AddTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
function(Add_module_test pixeltype)
  add_test(NAME AddTest_${pixeltype} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
    --compare ${BASELINE}/AddTest_${pixeltype}.mha
    ${TEMP}/AddTest_${pixeltype}.mha
    ModuleEntryPoint
    ${TEST_DATA}/brainSlice${pixeltype}.mha ${TEST_DATA}/brainSlice${pixeltype}.mha ${TEMP}/AddTest_${pixeltype}.mha
    )
  SET_PROPERTY(TEST AddTest_${pixeltype} PROPERTY LABELS ${CLP})
endfunction()
Add_module_test( CHAR )
Add_module_test( UCHAR )
Add_module_test( SHORT )
Add_module_test( USHORT )
Add_module_test( INT )
Add_module_test( UINT )
Add_module_test( LONG )
Add_module_test( ULONG )
Add_module_test( FLOAT )
Add_module_test( DOUBLE )

#-----------------------------------------------------------------------------
# Mask tests
#-----------------------------------------------------------------------------
set (CLP Mask)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/MaskedVolume.nrrd
  ${TEMP}/MaskedVolume.nrrd
  ModuleEntryPoint --label 1
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxialMask.nrrd ${TEMP}/MaskedVolume.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# Threshold tests
#-----------------------------------------------------------------------------
set (CLP Threshold)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/ThresholdTest.nhdr
  ${TEMP}/ThresholdTest.nhdr
  ModuleEntryPoint --upper 3800 --lower 1400 --outsidevalue 0 --thresholdtype Outside
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/ThresholdTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# Cast tests
#-----------------------------------------------------------------------------
set (CLP Cast)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

function(Cast_module_test pixeltype)
  add_test(NAME CastTest_${pixeltype} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
    --compare ${BASELINE}/CastTest${pixeltype}.nrrd
              ${TEMP}/CastTest${pixeltype}.nrrd
    ModuleEntryPoint --type ${pixeltype}
    ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTest${pixeltype}.nrrd
    )
  SET_PROPERTY(TEST CastTest_${pixeltype} PROPERTY LABELS Cast)
endfunction()
Cast_module_test( UnsignedChar )
Cast_module_test( Char )
Cast_module_test( Short )
Cast_module_test( UnsignedShort )
Cast_module_test( Int )
Cast_module_test( UnsignedInt )
Cast_module_test( Float )
Cast_module_test( Double )

#-----------------------------------------------------------------------------
# CheckerBoard tests
#-----------------------------------------------------------------------------
set (CLP CheckerBoard)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/CheckerBoardTest.nhdr
  ${TEMP}/CheckerBoardTest.nhdr
  ModuleEntryPoint --checkerPattern 5,3,2
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxialDoubled.nhdr ${TEMP}/CheckerBoardTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# Median tests
#-----------------------------------------------------------------------------
set (CLP MedianImageFilter)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/MedianImageFilterTest.nhdr
  ${TEMP}/MedianImageFilterTest.nhdr
  ModuleEntryPoint
  --neighborhood 1,2,3 ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/MedianImageFilterTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test2)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
  --help
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# GaussianBlur tests
#-----------------------------------------------------------------------------
set (CLP GaussianBlurImageFilter)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/GaussianBlurImageFilterTest.nhdr
  ${TEMP}/GaussianBlurImageFilterTest.nhdr
  ModuleEntryPoint
  --sigma 2.5 ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/GaussianBlurImageFilterTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# LinearRegistration tests
#-----------------------------------------------------------------------------
set (CLP LinearRegistration)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test00)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test01)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test02)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test03)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test04)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

if(SLICER_BRAINWEB_DATA_ROOT)
  set(testname ${CLP}Test05)
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
  set(testname ${CLP}Test06)
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
  set(testname ${CLP}Test07)
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
  set(testname ${CLP}Test08)
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
  set(testname ${CLP}Test09)
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
endif(SLICER_BRAINWEB_DATA_ROOT)

#-----------------------------------------------------------------------------
# BSplineDeformableRegistration tests
#-----------------------------------------------------------------------------
set (CLP BSplineDeformableRegistration)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test00)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

if(SLICER_BRAINWEB_DATA_ROOT)

  add_executable(BSplineWarping3DTest BSplineWarping3DTest.cxx)
  target_link_libraries(BSplineWarping3DTest ${ITK_LIBRARIES})
  SET_TARGET_PROPERTIES(BSplineWarping3DTest PROPERTIES LABELS ${CLP})
  
  set(testname BSplineWarping3DTest00)
  add_test(NAME ${testname} COMMAND $<TARGET_FILE:BSplineWarping3DTest>
    ${TEST_DATA}/BSplineDisplacements3D_00.txt
    ${SLICER_BRAINWEB_DATA_ROOT}/brainweb1e1a10f20.mha
    ${SLICER_BRAINWEB_DATA_ROOT}/brainweb1e1a10f20.mha
    ${TEMP}/BSplineWarping3DTest00.mhd
  )
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
  set(testname ${CLP}Test01)
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
  set(testname ${CLP}Test02)
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
  set(testname ${CLP}Test03)
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
  set(testname ${CLP}Test04)
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
  SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
endif(SLICER_BRAINWEB_DATA_ROOT)

#-----------------------------------------------------------------------------
# TestGridTransformRegistration tests
#-----------------------------------------------------------------------------
set (CLP TestGridTransformRegistration)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test00)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
  --gridSize 5 
  --outputtransform ${TEMP}/TestGridTransformRegistrationTest00Transform.txt 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  ${TEST_DATA}/CTHeadAxial.nhdr 
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# AffineRegistration tests
#-----------------------------------------------------------------------------
set (CLP AffineRegistration)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test000)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint  --help
  )

set(testname ${CLP}Test00)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test01)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test02)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# ConfidenceConnected tests
#-----------------------------------------------------------------------------
set (CLP ConfidenceConnected)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# CurvatureAnisotropicDiffusion tests
#-----------------------------------------------------------------------------
#set (CLP CurvatureAnisotropicDiffusion)

#add_executable(${CLP}Test ${CLP}Test.cxx)
#target_link_libraries(${CLP}Test ${CLP}Lib)

#set(testname ${CLP}Test)
#add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
#  --compare ${BASELINE}/${CLP}Test.nhdr
#            ${TEMP}/${CLP}Test.nhdr
#    ModuleEntryPoint
#    --conductance 2
#    --timeStep 0.0625
#    --iterations 2
#    ${TEST_DATA}/MRHeadResampled.nhdr
#    ${TEMP}/${CLP}Test.nhdr
#  )
#SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# DiffusionTensorEstimation tests
#-----------------------------------------------------------------------------
set (CLP DiffusionTensorEstimation)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries (${CLP}Test MRML vtkTeem SlicerBaseLogic)
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  DiffusionTensorEstimationTest
    --otsuomegathreshold 0.5
    ${MRML_TEST_DATA}/helix-DWI.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_dti.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_baseline.nhdr
    ${TEMP}/DiffusionTensorEstimationTest_otsu.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# DiffusionTensorMathematics tests
#-----------------------------------------------------------------------------
set (CLP DiffusionTensorMathematics)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test MRML vtkTeem SlicerBaseLogic)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  DiffusionTensorMathematicsTest
    --enumeration FractionalAnisotropy
    ${MRML_TEST_DATA}/helix-DTI.nhdr
    ${TEMP}/DiffusionTensorMathematicsTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# DiffusionTensorTest tests
#-----------------------------------------------------------------------------
set (CLP DiffusionTensorTest)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    ${MRML_TEST_DATA}/helix-DTI.nhdr
    ${TEMP}/DiffusionTensorTestTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# DiffusionWeightedTest tests
#-----------------------------------------------------------------------------
#set (CLP DiffusionWeightedTest)

#add_executable(${CLP}Test ${CLP}Test.cxx)
#target_link_libraries(${CLP}Test ${CLP}Lib)
#SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

#set(testname ${CLP}Test)
#add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
#  ModuleEntryPoint
#    ${MRML_TEST_DATA}/helix-DWI.nhdr
#    ${TEMP}/DiffusionWeightedTestTest.nhdr
#  )
#SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# DWIDicomLoad tests
#-----------------------------------------------------------------------------
# set (CLP DWIDicomLoad)

# add_executable(${CLP}Test ${CLP}Test.cxx)
# add_dependencies(${CLP}Test ${CLP})
# target_link_libraries(${CLP}Test ${ITK_LIBRARIES})
# SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

# set(testname ${CLP}Test)
# add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
#  DWIDicomLoadTest --help
#  )
#SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# ExecutionModelTour tests
#-----------------------------------------------------------------------------
set (CLP ExecutionModelTour)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test MRML)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# FreesurferSurfaceSectionExtraction tests
#-----------------------------------------------------------------------------
#set (CLP FreesurferSurfaceSectionExtraction)

#add_executable(${CLP}Test ${CLP}Test.cxx)
#add_dependencies(${CLP}Test ${CLP})
#target_link_libraries(${CLP}Test ${ITK_LIBRARIES})
#SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

#set(testname ${CLP}Test)
#add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
#  FreesurferSurfaceSectionExtractionTest --help
#  )
#SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# GradientAnisotropicDiffusion tests
#-----------------------------------------------------------------------------
set (CLP GradientAnisotropicDiffusion)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/GradientAnisotropicDiffusionTest.nhdr
            ${TEMP}/GradientAnisotropicDiffusionTest.nhdr
  ModuleEntryPoint
    --conductance 2
    --timeStep 0.0625
    --iterations 2
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GradientAnisotropicDiffusionTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# GrayscaleFillHoleImageFilter tests
#-----------------------------------------------------------------------------
set (CLP GrayscaleFillHoleImageFilter)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/GrayscaleFillHoleTest.nhdr
            ${TEMP}/GrayscaleFillHoleTest.nhdr
  ModuleEntryPoint
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GrayscaleFillHoleTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# GrayscaleGrindPeakImageFilter tests
#-----------------------------------------------------------------------------
set (CLP GrayscaleGrindPeakImageFilter)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/GrayscaleGrindPeakTest.nhdr
            ${TEMP}/GrayscaleGrindPeakTest.nhdr
  ModuleEntryPoint
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GrayscaleGrindPeakTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# GrayscaleModelMaker tests
#-----------------------------------------------------------------------------
set (CLP GrayscaleModelMaker)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkITK
  ${ITK_LIBRARIES}
  ModuleDescriptionParser
  SlicerBaseCLI
  vtkHybrid
  )
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# HistogramMatching tests
#-----------------------------------------------------------------------------
set (CLP HistogramMatching)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/HistogramMatchingTest.nhdr
            ${TEMP}/HistogramMatchingTest.nhdr
  ModuleEntryPoint
    --numberOfHistogramLevels 64
    --numberOfMatchPoints 10
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/HistogramMatchingTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# ImageLabelCombine tests
#-----------------------------------------------------------------------------
set (CLP ImageLabelCombine)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${ITK_LIBRARIES})
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ImageLabelCombineTest --help
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# ImageReadDicomWrite tests
#-----------------------------------------------------------------------------
set (CLP ImageReadDicomWrite)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# LabelMapSmoothing tests
#-----------------------------------------------------------------------------
set (CLP LabelMapSmoothing)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# ModelMaker tests
#-----------------------------------------------------------------------------
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
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

CONFIGURE_FILE(${TEST_DATA}/ModelMakerTest.mrml
    ${TEMP}/ModelMakerTest.mrml) 
    
set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModelMakerTest
    --labels 2
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helix-roi-lable2.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}GenerateAllOneLabelTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModelMakerTest
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helix-roi-lable2.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
set(testname ${CLP}GenerateAllThreeLabelsTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModelMakerTest
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}GenerateAllThreeLabelsPadTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModelMakerTest
    --generateAll
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    --pad
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})


set(testname ${CLP}StartEndTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModelMakerTest
    --start 1 --end 5
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}LabelsTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModelMakerTest
    --labels 3,1,5
    --modelSceneFile ${TEMP}/ModelMakerTest.mrml\#vtkMRMLModelHierarchyNode1
    ${MRML_TEST_DATA}/helixMask3Labels.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# MultipleModelsExample tests
#-----------------------------------------------------------------------------
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
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  MultipleModelsExampleTest --help
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# OtsuThresholdImageFilter tests
#-----------------------------------------------------------------------------
set (CLP OtsuThresholdImageFilter)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OtsuThresholdImageFilterTest.nhdr 
            ${TEMP}/OtsuThresholdImageFilterTest.nhdr 
   ModuleEntryPoint
    --insideValue 0
    --outsideValue 255
    --numberOfBins 128 
   ${TEST_DATA}/CTHeadResampled.nhdr
   ${TEMP}/OtsuThresholdImageFilterTest.nhdr 
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# OtsuThresholdSegmentation tests
#-----------------------------------------------------------------------------
set (CLP OtsuThresholdSegmentation)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# ResampleVolume2 tests
#-----------------------------------------------------------------------------
set (CLP ResampleVolume2)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(TRANSFORMS_DIR ${Slicer_SOURCE_DIR}/Applications/CLI/DiffusionApplications/ResampleDTI/Testing/TestData )

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(TransformFile ${TRANSFORMS_DIR}/rotation.tfm)
set(testname ${CLP}RotationNNTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(RotationAndAffineFile ${TRANSFORMS_DIR}/rotationAndAffine.tfm)
set(testname ${CLP}RotationAndAffineTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(BSplineFile ${Slicer_SOURCE_DIR}/Testing/Data/Input/MRHeadResampledBSplineTransform.tfm)
set(testname ${CLP}BSplineWSInterpolationTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(AffineFile ${TRANSFORMS_DIR}/affine.tfm)
set(testname ${CLP}BSplineInterpolationTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(HFieldFile ${Slicer_SOURCE_DIR}/Testing/Data/Input/MRHeadResampledHField.nrrd)
set(testname ${CLP}HFieldTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledHFieldTest.nrrd
    ${TEMP}/ResampleVolume2Test6.nrrd
  ModuleEntryPoint
    -H ${HFieldFile}
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test6.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# ResampleVolume tests
#-----------------------------------------------------------------------------
set (CLP ResampleVolume)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/ResampleVolumeTest.nhdr
            ${TEMP}/ResampleVolumeTest.nhdr
  ModuleEntryPoint
    --spacing 5,5,5
    --interpolation linear
   ${TEST_DATA}/MRHeadResampled.nhdr 
   ${TEMP}/ResampleVolumeTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# RigidRegistration tests
#-----------------------------------------------------------------------------
set (CLP RigidRegistration)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test000)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint --help
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test00)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test01)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test02)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test03)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test04)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

# This test reproduces exactly the registration step on
# the meningioma growth detection (tutorial) data for ChangeTracker
# Setting parameters explicitly, because default values are not passed to CLI
# In testing mode, this test is supposed to fail if the error at the image
# corner (as compared to the transform passed with --initialtransform) exceeds
# 0.1
#set(testname ${CLP}Test02)
#add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
#SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# Subtract tests
#-----------------------------------------------------------------------------
set (CLP Subtract)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/SubtractTest.nhdr
            ${TEMP}/SubtractTest.nhdr
  ModuleEntryPoint
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/CTHeadAxialDoubled.nhdr
    ${TEMP}/SubtractTest.nhdr
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# Multiply tests
#-----------------------------------------------------------------------------
set (CLP Multiply)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/MultiplyTest.nrrd
            ${TEMP}/MultiplyTest.nrrd
  ModuleEntryPoint
    --order 1
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEMP}/MultiplyTest.nrrd
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# VotingBinaryHoleFillingImageFilter tests
#-----------------------------------------------------------------------------
set (CLP VotingBinaryHoleFillingImageFilter)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# ZeroCrossingBasedEdgeDetectionImageFilter tests
#-----------------------------------------------------------------------------
#
#add_executable(${CLP}Test ${CLP}Test.cxx)
#target_link_libraries(${CLP}Test ${CLP}Lib)
#SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})
#
# culled from Slicer 3.6
#set (CLP ZeroCrossingBasedEdgeDetectionImageFilter)
#set(testname ${CLP}Test)
#add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
#  --compare ${BASELINE}/ZeroCrossingBasedEdgeDetectionImageFilterTest.nhdr
#            ${TEMP}/ZeroCrossingBasedEdgeDetectionImageFilterTest.nhdr
#  ModuleEntryPoint
#    ${TEST_DATA}/CTHeadResampled.nhdr
#    ${TEMP}/ZeroCrossingBasedEdgeDetectionImageFilterTest.nhdr
#  )
#SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# PolyDataToLabelmap tests
#-----------------------------------------------------------------------------
set (CLP PolyDataToLabelmap)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OAS10001.mha
            ${TEMP}/PolyDataToLabelmapTestOutput.mha
  --compareNumberOfPixelsTolerance 20
  ModuleEntryPoint
    ${TEST_DATA}/PolyDataToLabelmap/OAS10001.hdr 
    ${TEST_DATA}/PolyDataToLabelmap/OAS10001.vtp 
    ${TEMP}/PolyDataToLabelmapTestOutput.mha
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# SparseFieldLevelSetContour tests
#-----------------------------------------------------------------------------
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
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

# leaks from tcl file
# set(testname ${CLP}Test2)
# add_test(NAME ${testname} COMMAND ${Slicer_EXECUTABLE} 
#  --test-mode 
#  --script ${Slicer_SOURCE_DIR}/Applications/CLI/SparseFieldLevelSetContour/Testing/SparseFieldLevelSetContourTest.tcl  
#  ${TEST_DATA}/SparseFieldLevelSetContourTest.vtp 
#  ${TEMP}/SparseFieldLevelSetContourTest.vtp   
#  ${BASELINE}/SparseFieldLevelSetContourTest.vtp
#  )
#SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
# use a helper to compare the baseline and output, running it from the launcher to set up the paths

# the test code doesn't link on windows
if (NOT WIN32 AND NOT APPLE)
set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test> ${testname}
  --inputScene ${TEST_DATA}/SparseFieldLevelSetContourTest.vtp
  --contourpoints 132.059,-3.68759,-378.683
  --contourpoints 127.652,-16.0285,-381.549
  --contourpoints 120.15,-27.4267,-382.79
  --contourpoints 117.438,-26.4077,-394.14
  --contourpoints 128.782,-14.2639,-389.404
  --contourpoints 134.297,1.63338,-387.623
  --outputModel ${TEMP}/SparseFieldLevelSetContourTest.vtp
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

endif (NOT WIN32 AND NOT APPLE)

#-----------------------------------------------------------------------------
# MergeModels tests
#-----------------------------------------------------------------------------
set (CLP MergeModels)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  vtkIO
  ${ITK_LIBRARIES}
  vtkGraphics
  )
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  MergeModelsTest
    ${MRML_TEST_DATA}/sphere.vtk
    ${MRML_TEST_DATA}/cube.vtk
    ${TEMP}/sphereCube.vtk
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
  
set(testname ${CLP}TestVtp)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  MergeModelsTest
    ${MRML_TEST_DATA}/sphere.vtp
    ${MRML_TEST_DATA}/cube.vtp
    ${TEMP}/sphereCube.vtp
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

# add a test that compares the outline to a baseline .vtp
set(testname ${CLP}TestCompare)
add_test(NAME ${testname} COMMAND ${Launcher_Command} ${CMAKE_COMMAND}
  -Dtest_cmd=MergeModelsTest
  -Dtest_name=MergeModelsTest
  -Dmodel1=${MRML_TEST_DATA}/sphere.vtp
  -Dmodel2=${MRML_TEST_DATA}/cube.vtp
  -Doutput_model=${TEMP}/sphereCube.vtp
  -Doutput_baseline=${BASELINE}/sphereCube.vtp
  -Doutput_test=${TEMP}/sphereCube.vtp
  -P  ${Slicer_SOURCE_DIR}/Applications/CLI/Testing/run_MergeModelsTest.cmake
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
# ACPC Realign test
#-----------------------------------------------------------------------------
set(CLP ACPC)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test  RealignLib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

# configure the input mrml file to copy it to the binary TEMP dir so
# the test doesn't modify the source tree one
configure_file(
  ${TEST_DATA}/ACPCTest.mrml
  ${TEMP}/ACPCTest.mrml
  COPYONLY
  )

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
#  --acpc ${TEST_DATA}/ACPC.fcsv --midline ${TEST_DATA}/midsag.fcsv 
  --acpc 0.5,-21.5,-9.5 --acpc -2.5,4.5,-16.5 --midline 12.5,-93.5,7.5 --midline 12.5,-93.5,-12.5 --midline 5.5,-93.5,-25.5 --midline 9.5,-83.5,-22.5 --midline 9.5,-83.5,-12.5 --midline 10.5,-83.5,10.5 --midline 10.5,-83.5,26.5 --midline 7.5,-68.5,38.5 --midline 7.5,-68.5,13.5 --midline 6.5,-68.5,-9.5 --midline 5.5,-68.5,-30.5 --midline 5.5,-68.5,-48.5 --midline 3.5,-49.5,50.5 --midline 4.5,-49.5,25.5 --midline 3.5,-49.5,5.5 --midline 4.5,-49.5,-32.5 --midline 3.5,-34.5,54.5 --midline 1.5,-34.5,30.5 --midline 1.5,-34.5,17.5 --midline 3.5,-34.5,-22.5 --midline 1.5,-19.5,58.5 --midline 0.5,-19.5,34.5 --midline 0.5,-19.5,18.5 --midline 0.5,-19.5,-13.5 --midline 1.5,-19.5,-25.5 --midline -1.5,-4.5,62.5 --midline -2.5,-4.5,36.5 --midline -1.5,-4.5,19.5 --midline -1.5,-4.5,0.5 --midline -1.5,-4.5,-12.5 --midline -1.5,-4.5,-24.5 --midline -1.5,-4.5,-39.5 --midline -3.5,10.5,49.5 --midline -1.5,10.5,28.5 --midline -3.5,10.5,14.5 --midline -2.5,10.5,5.5 --midline -3.5,10.5,-15.5 --midline -1.5,10.5,-33.5 --midline -5.5,26.5,44.5 --midline -6.5,26.5,25.5 --midline -5.5,26.5,3.5 --midline -5.5,26.5,-11.5 --midline -5.5,26.5,-21.5 --midline -5.5,26.5,-35.5 --midline -5.5,41.5,35.5 --midline -7.5,41.5,12.5 --midline -6.5,41.5,-4.5 --midline -7.5,41.5,-27.5 --midline -7.5,41.5,-41.5 --midline -7.5,56.5,24.5 --midline -9.5,56.5,-0.5 --midline -9.5,56.5,-12.5 --midline -9.5,56.5,-29.5 --midline -10.5,71.5,3.5 --midline -10.5,71.5,-12.5 --midline -11.5,71.5,-26.5 --outputTransform ${TEMP}/ACPCTest.mrml\#vtkMRMLLinearTransformNode1
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

