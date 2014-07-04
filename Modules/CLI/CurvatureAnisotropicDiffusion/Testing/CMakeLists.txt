
#-----------------------------------------------------------------------------
#set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
#set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
#add_executable(${CLP}Test ${CLP}Test.cxx)
#target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})

#set(testname ${CLP}Test)
#add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
#  --compare ${BASELINE}/${CLP}Test.nhdr
#            ${TEMP}/${CLP}Test.nhdr
#    ModuleEntryPoint
#    --conductance 2
#    --timeStep 0.0625
#    --iterations 2
#    ${TEST_DATA}/MRHeadResampled.nhdr
#    ${TEMP}/${CLP}Test.nhdr
#  )
#set_property(TEST ${testname} PROPERTY LABELS ${CLP})
