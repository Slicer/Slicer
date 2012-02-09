set(KIT ColorsLogic)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkSlicerColorLogicTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${lib_name} vtkSlicerColorsModuleLogic)

macro(SIMPLE_TEST  TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
endmacro()

SIMPLE_TEST( vtkSlicerColorLogicTest1 )

