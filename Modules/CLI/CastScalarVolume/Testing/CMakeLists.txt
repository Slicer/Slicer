
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

function(add_module_test pixeltype)
  set(testname ${CLP}Test_${pixeltype})
  ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
    NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
    --compare DATA{${BASELINE}/${CLP}Test${pixeltype}.nrrd}
              ${TEMP}/${CLP}Test${pixeltype}.nrrd
    ModuleEntryPoint --type ${pixeltype}
    DATA{${INPUT}/${CLP}Test.nrrd} ${TEMP}/${CLP}Test${pixeltype}.nrrd
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

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
