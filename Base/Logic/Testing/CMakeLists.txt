set(KIT BaseLogic)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkArchiveTest1.cxx
  vtkDataIOManagerLogicTest1.cxx
  vtkSlicerTransformLogicTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
)

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${PROJECT_NAME})
add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${lib_name})
set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${LIBRARY_NAME})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
  set_property(TEST ${TESTNAME} PROPERTY LABELS ${LIBRARY_NAME})
endmacro()

SIMPLE_TEST( vtkArchiveTest1 ${CMAKE_CURRENT_SOURCE_DIR}/vol.zip)
SIMPLE_TEST( vtkDataIOManagerLogicTest1 )
SIMPLE_TEST( vtkSlicerTransformLogicTest1 ${CMAKE_CURRENT_SOURCE_DIR}/affineTransform.txt)
