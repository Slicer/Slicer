
#-----------------------------------------------------------------------------
# Multiply tests
#-----------------------------------------------------------------------------
set (CLP Multiply)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

