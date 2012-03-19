set(KIT ColorsLogic)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkSlicerColorLogicTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${lib_name} vtkSlicerColorsModuleLogic)

SIMPLE_TEST( vtkSlicerColorLogicTest1 )

