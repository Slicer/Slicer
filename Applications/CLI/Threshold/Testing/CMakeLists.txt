
#-----------------------------------------------------------------------------
# Threshold tests
#-----------------------------------------------------------------------------
set (CLP Threshold)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/ThresholdTest.nhdr
  ${TEMP}/ThresholdTest.nhdr
  ModuleEntryPoint --upper 3800 --lower 1400 --outsidevalue 0 --thresholdtype Outside
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/ThresholdTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

