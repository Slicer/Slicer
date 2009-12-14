
SET(KIT QTCore)
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerCoreApplicationTest1.cxx
  qSlicerCoreIOManagerTest1.cxx
  qSlicerModuleFactoryTest1.cxx
  qSlicerModuleManagerTest1.cxx
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

SET( QTCore_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${QTCore_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )


SIMPLE_TEST( qSlicerCoreApplicationTest1 )
SIMPLE_TEST( qSlicerCoreIOManagerTest1 )
SIMPLE_TEST( qSlicerModuleFactoryTest1 )
SIMPLE_TEST( qSlicerModuleManagerTest1 )

