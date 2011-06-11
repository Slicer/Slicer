SET(KIT QTGUI)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
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

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})
SET_TARGET_PROPERTIES(${KIT}CxxTests PROPERTIES LABELS ${LIBRARY_NAME})

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME})
  SET_PROPERTY(TEST ${TESTNAME} PROPERTY LABELS ${LIBRARY_NAME})
ENDMACRO()

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
