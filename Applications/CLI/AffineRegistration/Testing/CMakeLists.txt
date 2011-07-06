
#-----------------------------------------------------------------------------
# AffineRegistration tests
#-----------------------------------------------------------------------------
set (CLP AffineRegistration)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

