set(KIT vtkSegmentationCore)

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkSegmentationTest1.cxx
  vtkSegmentationConverterTest1.cxx
  vtkClosedSurfaceToFractionalLabelMapConversionTest1.cxx
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${PROJECT_NAME})
set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

macro(TEST_FILE TEST_NAME FILENAME)
  add_test(
    NAME ${TEST_NAME}_${SCENEFILENAME}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TEST_NAME}
      ${FILENAME}
    )
endmacro()

simple_test( vtkSegmentationTest1 )
simple_test( vtkSegmentationConverterTest1 )
simple_test( vtkClosedSurfaceToFractionalLabelMapConversionTest1 )
