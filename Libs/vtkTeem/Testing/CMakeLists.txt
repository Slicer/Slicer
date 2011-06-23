set(KIT vtkTeem)

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkDiffusionTensorMathematicsTest1.cxx
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${PROJECT_NAME})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${lib_name})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
endmacro()

macro(TEST_FILE TEST_NAME FILENAME)
  add_test(
    NAME ${TEST_NAME}_${SCENEFILENAME}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TEST_NAME}
      ${FILENAME}
    )
endmacro()

SIMPLE_TEST( vtkDiffusionTensorMathematicsTest1 )
