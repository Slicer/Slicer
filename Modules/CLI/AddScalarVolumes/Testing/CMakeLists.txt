
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(CLP ${MODULE_NAME})

if(NOT DEFINED SEM_DATA_MANAGEMENT_TARGET)
  set(SEM_DATA_MANAGEMENT_TARGET ${CLP}Data)
endif()

#-----------------------------------------------------------------------------
ctk_add_executable_utf8(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare DATA{${BASELINE}/${CLP}Test.nhdr,AddScalarVolumesTest.raw}
  ${TEMP}/${CLP}Test.nhdr
  ModuleEntryPoint
  DATA{${INPUT}/CTHeadAxial.nhdr,CTHeadAxial.raw.gz,CTHeadAxial.raw.gz} DATA{${INPUT}/CTHeadAxial.nhdr,CTHeadAxial.raw.gz,CTHeadAxial.raw.gz} ${TEMP}/${CLP}Test.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
function(add_module_test pixeltype)
  set(testname ${CLP}Test_${pixeltype})
  ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
    NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
    --compare DATA{${BASELINE}/${CLP}Test_${pixeltype}.mha}
    ${TEMP}/${CLP}Test_${pixeltype}.mha
    ModuleEntryPoint
    DATA{${INPUT}/brainSlice${pixeltype}.mha}
    DATA{${INPUT}/brainSlice${pixeltype}.mha}
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

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
