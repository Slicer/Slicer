set(KIT vtkTeem)

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkDiffusionTensorMathematicsTest1.cxx
  )

set(LIBRARY_NAME ${PROJECT_NAME})

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${lib_name})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

simple_test( vtkDiffusionTensorMathematicsTest1 )
