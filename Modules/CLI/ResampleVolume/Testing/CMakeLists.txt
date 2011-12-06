
#-----------------------------------------------------------------------------
# ResampleVolume tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP ResampleVolume)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/ResampleVolumeTest.nhdr
            ${TEMP}/ResampleVolumeTest.nhdr
  ModuleEntryPoint
    --spacing 5,5,5
    --interpolation linear
   ${TEST_DATA}/MRHeadResampled.nhdr
   ${TEMP}/ResampleVolumeTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

