
#-----------------------------------------------------------------------------
# GrayscaleFillHoleImageFilter tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP GrayscaleFillHoleImageFilter)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/GrayscaleFillHoleTest.nhdr
            ${TEMP}/GrayscaleFillHoleTest.nhdr
  ModuleEntryPoint
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/GrayscaleFillHoleTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

