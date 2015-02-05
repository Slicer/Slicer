
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

function(add_module_test pixeltype)
  set(testname ${CLP}Test_${pixeltype})
  add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
    --compare ${BASELINE}/${CLP}Test${pixeltype}.nrrd
              ${TEMP}/${CLP}Test${pixeltype}.nrrd
    ModuleEntryPoint --type ${pixeltype}
    ${INPUT}/${CLP}Test.nrrd ${TEMP}/${CLP}Test${pixeltype}.nrrd
    )
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})
endfunction()
add_module_test( UnsignedChar )
add_module_test( Char )
add_module_test( Short )
add_module_test( UnsignedShort )
add_module_test( Int )
add_module_test( UnsignedInt )
add_module_test( Float )
add_module_test( Double )

