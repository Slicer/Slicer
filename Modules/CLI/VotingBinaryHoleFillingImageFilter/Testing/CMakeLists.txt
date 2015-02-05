
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/VotingBinaryHoleFillingImageFilterTest.nhdr
            ${TEMP}/VotingBinaryHoleFillingImageFilterTest.nhdr
  ModuleEntryPoint
    --radius 3,3,3
    --majorityThreshold 1
    --background 0
    --foreground 1
   ${TEST_DATA}/CTHeadResampledOtsuSegmented.nhdr
   ${TEMP}/VotingBinaryHoleFillingImageFilterTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
