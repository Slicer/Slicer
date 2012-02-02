
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP MultiplyScalarVolumes)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/${CLP}Test.nrrd
            ${TEMP}/${CLP}Test.nrrd
  ModuleEntryPoint
    --order 1
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEMP}/${CLP}Test.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

