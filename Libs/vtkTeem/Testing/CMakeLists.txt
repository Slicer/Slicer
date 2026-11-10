set(KIT vtkTeem)

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkDiffusionTensorMathematicsTest1.cxx
  )

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests PRIVATE Slicer::Teem)

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER vtkTeem)

simple_test( vtkDiffusionTensorMathematicsTest1 )
