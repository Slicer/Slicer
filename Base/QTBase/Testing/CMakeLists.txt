
SET(KIT QTBase)
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerAbstractCoreModuleTest1.cxx
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

SET( QTBase_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${QTBase_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )


SIMPLE_TEST( qSlicerAbstractCoreModuleTest1 )

