SET(KIT QTCore)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerCoreApplicationTest1.cxx
  qSlicerCoreIOManagerTest1.cxx
  qSlicerLoadableModuleFactoryTest1.cxx
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

# Remark: qSlicerModuleFactoryManager class is tested within Applications/SlicerQT/Testing

ADD_TEST( qSlicerCoreApplicationTest1 
  ${LAUNCH_EXE}
  ${QTCore_TESTS} qSlicerCoreApplicationTest1 
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SlicerQT )

ADD_TEST( qSlicerCoreIOManagerTest1 
  ${LAUNCH_EXE}
  ${QTCore_TESTS} qSlicerCoreIOManagerTest1 
  ${CMAKE_SOURCE_DIR}/Libs/MRML/Testing/name.mrml)

SIMPLE_TEST( qSlicerLoadableModuleFactoryTest1 )
