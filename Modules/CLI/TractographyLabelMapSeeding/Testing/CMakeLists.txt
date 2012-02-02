set(module_name TractographyLabelMapSeeding)

add_executable(${module_name}Test ${module_name}Test.cxx)
add_dependencies(${module_name}Test ${module_name})
target_link_libraries(${module_name}Test
  vtkITK
  ${ITK_LIBRARIES}
  vtkTeem
  ModuleDescriptionParser
  SlicerBaseCLI
)

set_target_properties(${module_name}Test PROPERTIES LABELS ${module_name})

set(testname ${module_name}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${module_name}Test>
  ${module_name}Test
  --outputdirectory ${TEMP}
  --name ${module_name}Test_line
  --seedspacing 4
  --clthreshold 0.3
  --minimumlength 10
  --maximumlength 800
  --stoppingmode LinearMeasure
  --stoppingvalue 0.1
  --stoppingcurvature 0.8
  --integrationsteplength 0.5
  --label 1
  ${MRML_TEST_DATA}/helix-DTI.nhdr
  ${TEMP}/${module_name}Test_helixTracts.vtp
  )
set_property(TEST ${testname} PROPERTY LABELS ${module_name})

