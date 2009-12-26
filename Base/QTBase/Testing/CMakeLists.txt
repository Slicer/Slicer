SET(KIT QTBase)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerAbstractCoreModuleTest1.cxx
  qSlicerAbstractLoadableModuleTest1.cxx
  qSlicerAbstractModuleTest1.cxx
  qSlicerAbstractModuleWidgetTest1.cxx
  qSlicerModuleLogicTest1.cxx
  qSlicerUtilsTest1.cxx
  qSlicerWidgetTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME} vtkCommon)

SET( QTBase_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${QTBase_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )


SIMPLE_TEST( qSlicerAbstractCoreModuleTest1 )
SIMPLE_TEST( qSlicerAbstractLoadableModuleTest1 )
SIMPLE_TEST( qSlicerAbstractModuleTest1 )
SIMPLE_TEST( qSlicerAbstractModuleWidgetTest1 )
SIMPLE_TEST( qSlicerModuleLogicTest1 )
SIMPLE_TEST( qSlicerUtilsTest1 )
SIMPLE_TEST( qSlicerWidgetTest1 )
