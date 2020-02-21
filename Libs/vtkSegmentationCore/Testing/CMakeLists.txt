set(KIT vtkSegmentationCore)

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkSegmentationTest1.cxx
  vtkSegmentationTest2.cxx
  vtkSegmentationHistoryTest1.cxx
  vtkSegmentationConverterTest1.cxx
  vtkClosedSurfaceToFractionalLabelMapConversionTest1.cxx
  )

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${PROJECT_NAME})
set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

simple_test( vtkSegmentationTest1 )
simple_test( vtkSegmentationTest2 )
simple_test( vtkSegmentationHistoryTest1 )
simple_test( vtkSegmentationConverterTest1 )
simple_test( vtkClosedSurfaceToFractionalLabelMapConversionTest1 )
