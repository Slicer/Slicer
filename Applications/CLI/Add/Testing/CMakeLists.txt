
#-----------------------------------------------------------------------------
# Add tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP Add)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/AddTest.nhdr
  ${TEMP}/AddTest.nhdr
  ModuleEntryPoint
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxial.nhdr ${TEMP}/AddTest.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
function(Add_module_test pixeltype)
  add_test(NAME AddTest_${pixeltype} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
    --compare ${BASELINE}/AddTest_${pixeltype}.mha
    ${TEMP}/AddTest_${pixeltype}.mha
    ModuleEntryPoint
    ${TEST_DATA}/brainSlice${pixeltype}.mha ${TEST_DATA}/brainSlice${pixeltype}.mha ${TEMP}/AddTest_${pixeltype}.mha
    )
  set_property(TEST AddTest_${pixeltype} PROPERTY LABELS ${CLP})
endfunction()
Add_module_test( CHAR )
Add_module_test( UCHAR )
Add_module_test( SHORT )
Add_module_test( USHORT )
Add_module_test( INT )
Add_module_test( UINT )
Add_module_test( LONG )
Add_module_test( ULONG )
Add_module_test( FLOAT )
Add_module_test( DOUBLE )
