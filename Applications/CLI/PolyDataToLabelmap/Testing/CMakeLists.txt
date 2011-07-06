
#-----------------------------------------------------------------------------
# PolyDataToLabelmap tests
#-----------------------------------------------------------------------------
set (CLP PolyDataToLabelmap)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

