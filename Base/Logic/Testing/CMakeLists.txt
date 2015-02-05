set(KIT ${PROJECT_NAME})

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
set(KIT_TEST_SRCS
  vtkDataIOManagerLogicTest1.cxx
  vtkSlicerApplicationLogicTest1.cxx
  vtkSlicerTransformLogicTest1.cxx
  vtkArchiveTest1.cxx
  )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})
set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${KIT})
set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Core-Base")

simple_test( vtkArchiveTest1 ${CMAKE_CURRENT_SOURCE_DIR}/vol.zip)
simple_test( vtkDataIOManagerLogicTest1 )
simple_test( vtkSlicerApplicationLogicTest1 )
simple_test( vtkSlicerTransformLogicTest1 ${CMAKE_CURRENT_SOURCE_DIR}/affineTransform.txt)
