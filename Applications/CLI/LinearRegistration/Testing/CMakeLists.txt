
#-----------------------------------------------------------------------------
# LinearRegistration tests
#-----------------------------------------------------------------------------
set (CLP LinearRegistration)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})

endif(SLICER_BRAINWEB_DATA_ROOT)

