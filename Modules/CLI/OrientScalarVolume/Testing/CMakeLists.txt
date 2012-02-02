
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP OrientScalarVolume)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

function(add_module_test orientation)
  set(testname ${CLP}Test${orientation})
  add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
    --compare ${BASELINE}/${testname}.nrrd
    ${TEMP}/${testname}.nrrd
    ModuleEntryPoint
    --orientation ${orientation} ${MRML_TEST_DATA}/fixed.nrrd ${TEMP}/${testname}.nrrd
    )
  set_property(TEST ${testname} PROPERTY LABELS ${CLP})
endfunction()

add_module_test(Axial)
add_module_test(Coronal)
add_module_test(Sagittal)
