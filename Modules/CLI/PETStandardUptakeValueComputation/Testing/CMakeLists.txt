get_filename_component(full_path_without_filename ${CMAKE_CURRENT_LIST_FILE} PATH)
get_filename_component(full_path_without_testing ${full_path_without_filename} PATH)
get_filename_component(module_name ${full_path_without_testing} NAME)

add_executable(${module_name}Test ${module_name}Test.cxx)
add_dependencies(${module_name}Test ${module_name})
target_link_libraries(${module_name}Test
  vtkITK
  ModuleDescriptionParser
  SlicerBaseCLI
  MRMLCore
)

set_target_properties(${module_name}Test PROPERTIES LABELS ${module_name})

set(testname ${module_name}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${module_name}Test>
  ${module_name}Test
  --xml
)
set_property(TEST ${testname} PROPERTY LABELS ${module_name})

