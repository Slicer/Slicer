
#-----------------------------------------------------------------------------
# OtsuThresholdSegmentation tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP OtsuThresholdSegmentation)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OtsuThresholdSegmentationTest.nhdr
            ${TEMP}/OtsuThresholdSegmentationTest.nhdr
  ModuleEntryPoint
    --brightObjects
    --numberOfBins 128
    --faceConnected
    --minimumObjectSize 1000
   ${TEST_DATA}/CTHeadResampled.nhdr
   ${TEMP}/OtsuThresholdSegmentationTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

