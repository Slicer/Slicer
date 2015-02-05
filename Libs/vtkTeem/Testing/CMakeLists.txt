set(KIT vtkTeem)

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkDiffusionTensorMathematicsTest1.cxx
  )

set(LIBRARY_NAME ${PROJECT_NAME})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${lib_name})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

macro(TEST_FILE TEST_NAME FILENAME)
  add_test(
    NAME ${TEST_NAME}_${SCENEFILENAME}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TEST_NAME}
      ${FILENAME}
    )
endmacro()

simple_test( vtkDiffusionTensorMathematicsTest1 )
