
#-----------------------------------------------------------------------------
# Cast tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP Cast)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

function(Cast_module_test pixeltype)
  add_test(NAME CastTest_${pixeltype} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
    --compare ${BASELINE}/CastTest${pixeltype}.nrrd
              ${TEMP}/CastTest${pixeltype}.nrrd
    ModuleEntryPoint --type ${pixeltype}
    ${TEST_DATA}/CastTest.nrrd ${TEMP}/CastTest${pixeltype}.nrrd
    )
  set_property(TEST CastTest_${pixeltype} PROPERTY LABELS Cast)
endfunction()
Cast_module_test( UnsignedChar )
Cast_module_test( Char )
Cast_module_test( Short )
Cast_module_test( UnsignedShort )
Cast_module_test( Int )
Cast_module_test( UnsignedInt )
Cast_module_test( Float )
Cast_module_test( Double )

