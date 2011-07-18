
#-----------------------------------------------------------------------------
# HistogramMatching tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP HistogramMatching)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/HistogramMatchingTest.nhdr
            ${TEMP}/HistogramMatchingTest.nhdr
  ModuleEntryPoint
    --numberOfHistogramLevels 64
    --numberOfMatchPoints 10
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/HistogramMatchingTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

