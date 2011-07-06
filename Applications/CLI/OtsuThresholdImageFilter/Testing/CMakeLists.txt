
#-----------------------------------------------------------------------------
# OtsuThresholdImageFilter tests
#-----------------------------------------------------------------------------
set (CLP OtsuThresholdImageFilter)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OtsuThresholdImageFilterTest.nhdr
            ${TEMP}/OtsuThresholdImageFilterTest.nhdr
   ModuleEntryPoint
    --insideValue 0
    --outsideValue 255
    --numberOfBins 128
   ${TEST_DATA}/CTHeadResampled.nhdr
   ${TEMP}/OtsuThresholdImageFilterTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

