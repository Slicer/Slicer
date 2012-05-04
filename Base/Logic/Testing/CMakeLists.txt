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

SIMPLE_TEST( vtkArchiveTest1 ${CMAKE_CURRENT_SOURCE_DIR}/vol.zip)
SIMPLE_TEST( vtkDataIOManagerLogicTest1 )
SIMPLE_TEST( vtkSlicerApplicationLogicTest1 )
SIMPLE_TEST( vtkSlicerTransformLogicTest1 ${CMAKE_CURRENT_SOURCE_DIR}/affineTransform.txt)
