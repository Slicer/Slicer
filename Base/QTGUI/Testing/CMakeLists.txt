set(KIT QTGUI)
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  qSlicerAbstractModuleTest1.cxx
  qSlicerAbstractModuleWidgetTest1.cxx
  qSlicerApplicationTest1.cxx
  qSlicerDirectoryListViewTest1.cxx
  qSlicerExtensionsWizardTest1.cxx
  qSlicerLayoutManagerTest1.cxx
  qSlicerLoadableModuleTest1.cxx
  qSlicerModulePanelTest1.cxx
  qSlicerWidgetTest1.cxx
  qSlicerWidgetTest2.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${PROJECT_NAME})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})
set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${LIBRARY_NAME})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
  set_property(TEST ${TESTNAME} PROPERTY LABELS ${LIBRARY_NAME})
endmacro()

#
# Add Tests
#

SIMPLE_TEST( qSlicerApplicationTest1 )
SIMPLE_TEST( qSlicerAbstractModuleTest1 )
SIMPLE_TEST( qSlicerAbstractModuleWidgetTest1 )
SIMPLE_TEST( qSlicerDirectoryListViewTest1 )
SIMPLE_TEST( qSlicerExtensionsWizardTest1 )
SIMPLE_TEST( qSlicerLayoutManagerTest1 )
SIMPLE_TEST( qSlicerLoadableModuleTest1 )
SIMPLE_TEST( qSlicerModulePanelTest1 )
SIMPLE_TEST( qSlicerWidgetTest1 )
SIMPLE_TEST( qSlicerWidgetTest2 )
