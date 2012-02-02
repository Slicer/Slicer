
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
set(CLP ModelToLabelMap)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OAS10001.mha
            ${TEMP}/${CLP}TestOutput.mha
  --compareNumberOfPixelsTolerance 20
  ModuleEntryPoint
    ${INPUT}/OAS10001.hdr
    ${INPUT}/OAS10001.vtp
    ${TEMP}/${CLP}TestOutput.mha
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

