set(KIT SlicerQT)
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  qSlicerAboutDialogTest1.cxx
  qSlicerMainWindowCoreTest1.cxx
  qSlicerMainWindowTest1.cxx
  qSlicerModuleFactoryManagerTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME qSlicerQT)

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
endmacro()

#
# Add Tests
#

SIMPLE_TEST( qSlicerAboutDialogTest1 )
SIMPLE_TEST( qSlicerMainWindowCoreTest1 )
SIMPLE_TEST( qSlicerMainWindowTest1 )
SIMPLE_TEST( qSlicerModuleFactoryManagerTest1 )

