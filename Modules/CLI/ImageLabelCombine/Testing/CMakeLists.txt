
#-----------------------------------------------------------------------------
# ImageLabelCombine tests
#-----------------------------------------------------------------------------
set (CLP ImageLabelCombine)

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint --help
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

