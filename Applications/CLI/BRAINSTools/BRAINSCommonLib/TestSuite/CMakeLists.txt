

#configure_file(${BRAINSCommonLibProject_SOURCE_DIR}/CTestCustom.ctest ${BRAINSCommonLibProject_BINARY_DIR}/CTestCustom.ctest COPYONLY)

add_executable(PrettyPrintTableTest PrettyPrintTableTest.cxx)
set_target_properties(PrettyPrintTableTest
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY
  ${BRAINSCommonLibProject_BINARY_DIR})

add_test(NAME PrettyPrintTableTest COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:PrettyPrintTableTest>)

