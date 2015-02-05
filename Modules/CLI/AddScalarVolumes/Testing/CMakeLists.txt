
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/${CLP}Test.nhdr
  ${TEMP}/${CLP}Test.nhdr
  ModuleEntryPoint
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/${CLP}Test.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
function(add_module_test pixeltype)
  set(testname ${CLP}Test_${pixeltype})
  add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
    --compare ${BASELINE}/${CLP}Test_${pixeltype}.mha
    ${TEMP}/${CLP}Test_${pixeltype}.mha
    ModuleEntryPoint
    ${TEST_DATA}/brainSlice${pixeltype}.mha
    ${TEST_DATA}/brainSlice${pixeltype}.mha
    ${TEMP}/${CLP}Test_${pixeltype}.mha
    )
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})
endfunction()
add_module_test( CHAR )
add_module_test( UCHAR )
add_module_test( SHORT )
add_module_test( USHORT )
add_module_test( INT )
add_module_test( UINT )
add_module_test( LONG )
add_module_test( ULONG )
add_module_test( FLOAT )
add_module_test( DOUBLE )
