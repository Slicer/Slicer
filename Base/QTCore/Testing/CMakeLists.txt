
SET(KIT QTCore)
SET(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "CHECK_FOR_VTK_MEMORY_LEAKS();")
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerCoreApplicationTest1.cxx
  qSlicerCoreIOManagerTest1.cxx
  qSlicerModuleFactoryTest1.cxx
  qSlicerModuleManagerTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
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


ADD_TEST( qSlicerCoreApplicationTest1 
  ${QTCore_TESTS} qSlicerCoreApplicationTest1 
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SlicerQT )

ADD_TEST( qSlicerCoreIOManagerTest1 
  ${QTCore_TESTS} qSlicerCoreIOManagerTest1 
  ${PROJECT_SOURCE_DIR}/Libs/MRML/Testing/name.mrml)

SIMPLE_TEST( qSlicerModuleFactoryTest1 )
SIMPLE_TEST( qSlicerModuleManagerTest1 )

