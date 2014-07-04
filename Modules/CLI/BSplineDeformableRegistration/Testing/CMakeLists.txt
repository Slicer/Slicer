
#-----------------------------------------------------------------------------
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test00)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

if(SLICER_BRAINWEB_DATA_ROOT)

  add_executable(BSplineWarping3DTest BSplineWarping3DTest.cxx)
  target_link_libraries(BSplineWarping3DTest ${ITK_LIBRARIES})
  set_target_properties(BSplineWarping3DTest PROPERTIES LABELS ${CLP})

  set(testname BSplineWarping3DTest00)
  add_test(NAME ${testname} COMMAND $<TARGET_FILE:BSplineWarping3DTest>
    ${TEST_DATA}/BSplineDisplacements3D_00.txt
    ${SLICER_BRAINWEB_DATA_ROOT}/brainweb1e1a10f20.mha
    ${SLICER_BRAINWEB_DATA_ROOT}/brainweb1e1a10f20.mha
    ${TEMP}/BSplineWarping3DTest00.mhd
  )
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})

  set(testname ${CLP}Test01)
  add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
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
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})

  set(testname ${CLP}Test02)
  add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
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
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})

  set(testname ${CLP}Test03)
  add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
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
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})

  set(testname ${CLP}Test04)
  add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
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
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})
endif(SLICER_BRAINWEB_DATA_ROOT)

