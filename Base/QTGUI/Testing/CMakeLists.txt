SET(KIT QTGUI)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerAbstractModuleTest1.cxx
  qSlicerAbstractModuleWidgetTest1.cxx
  qSlicerLayoutManagerTest1.cxx
  qSlicerLayoutManagerTest2.cxx
  qSlicerLoadableModuleTest1.cxx
  qSlicerWidgetTest1.cxx
  qSlicerWidgetTest2.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

SET( KIT_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)
IF(WIN32)
  SET(KIT_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${KIT_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )

#
# Add Tests
#

# ADD_TEST( qSlicerCoreApplicationTest1
#   ${LAUNCH_EXE}
#   ${QTCore_TESTS} qSlicerCoreApplicationTest1
#   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SlicerQT )

SIMPLE_TEST( qSlicerAbstractModuleTest1 )
SIMPLE_TEST( qSlicerAbstractModuleWidgetTest1 )
SIMPLE_TEST( qSlicerLayoutManagerTest1 )
SIMPLE_TEST( qSlicerLayoutManagerTest2 )
SIMPLE_TEST( qSlicerLoadableModuleTest1 )
SIMPLE_TEST( qSlicerWidgetTest1 )
SIMPLE_TEST( qSlicerWidgetTest2 )
