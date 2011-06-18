SET(KIT SlicerQT)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerAboutDialogTest1.cxx
  qSlicerMainWindowCoreTest1.cxx
  qSlicerMainWindowTest1.cxx
  qSlicerModuleFactoryManagerTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME qSlicerQT)

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
ENDMACRO()

#
# Add Tests
#

SIMPLE_TEST( qSlicerAboutDialogTest1 )
SIMPLE_TEST( qSlicerMainWindowCoreTest1 )
SIMPLE_TEST( qSlicerMainWindowTest1 )
SIMPLE_TEST( qSlicerModuleFactoryManagerTest1 )

