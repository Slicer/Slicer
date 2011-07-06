
#-----------------------------------------------------------------------------
# Subtract tests
#-----------------------------------------------------------------------------
set (CLP Subtract)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/SubtractTest.nhdr
            ${TEMP}/SubtractTest.nhdr
  ModuleEntryPoint
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/CTHeadAxialDoubled.nhdr
    ${TEMP}/SubtractTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

