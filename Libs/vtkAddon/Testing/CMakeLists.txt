set(KIT vtkAddon)

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkAddonMathUtilitiesTest1.cxx
  vtkAddonTestingUtilitiesTest1.cxx
  vtkLoggingMacrosTest1.cxx
  vtkPersonInformationTest1.cxx
  )

set(LIBRARY_NAME ${PROJECT_NAME})

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${lib_name})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

simple_test( vtkAddonMathUtilitiesTest1 )
simple_test( vtkAddonTestingUtilitiesTest1 )
simple_test( vtkLoggingMacrosTest1 )
simple_test( vtkPersonInformationTest1 )
