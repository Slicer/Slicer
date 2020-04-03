set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(KIT ${PROJECT_NAME})

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
set(KIT_TEST_SRCS
  vtkDataIOManagerLogicTest1.cxx
  vtkSlicerApplicationLogicTest1.cxx
  vtkSlicerVersionConfigureTest1.cxx
  )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})
set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${KIT})
set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Core-Base")

simple_test( vtkDataIOManagerLogicTest1 )
simple_test( vtkSlicerApplicationLogicTest1 )
simple_test( vtkSlicerVersionConfigureTest1 )
