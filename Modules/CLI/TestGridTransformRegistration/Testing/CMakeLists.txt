
#-----------------------------------------------------------------------------
# TestGridTransformRegistration tests
#-----------------------------------------------------------------------------
set (CLP TestGridTransformRegistration)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test00)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
  --gridSize 5
  --outputtransform ${TEMP}/TestGridTransformRegistrationTest00Transform.txt
  ${TEST_DATA}/CTHeadAxial.nhdr
  ${TEST_DATA}/CTHeadAxial.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

