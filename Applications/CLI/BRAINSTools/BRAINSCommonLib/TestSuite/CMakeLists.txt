
if (WIN32)
  set(BUILD_SUBDIR ${CMAKE_BUILD_TYPE}/)
endif (WIN32)

#configure_file(${BRAINSCommonLibProject_SOURCE_DIR}/CTestCustom.ctest ${BRAINSCommonLibProject_BINARY_DIR}/CTestCustom.ctest COPYONLY)

add_executable(PrettyPrintTableTest PrettyPrintTableTest.cxx)
set_target_properties(PrettyPrintTableTest
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY
  ${BRAINSCommonLibProject_BINARY_DIR})

set(PrettyPrintTableTest_EXEC ${LAUNCH_EXE} ${BRAINSCommonLibProject_BINARY_DIR}/${BUILD_SUBDIR}PrettyPrintTableTest)

add_test(PrettyPrintTableTest ${PrettyPrintTableTest_EXEC} )

