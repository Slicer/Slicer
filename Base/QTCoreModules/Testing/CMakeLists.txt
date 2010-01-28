SET(KIT QTCoreModules)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerCamerasModuleTest1.cxx
  qSlicerCamerasModuleWidgetTest1.cxx
  qSlicerTransformsModuleLogicTest1.cxx
  qSlicerTransformsModuleTest1.cxx
  qSlicerTransformsModuleWidgetTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests 
  ${lib_name} 
  qSlicerBaseQTBase  # FIXME: This shouldn't be necessary: there is a library dependency problem
  qSlicerBaseQTCore  # FIXME: This shouldn't be necessary: there is a library dependency problem
  )

SET( QTCoreModules_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

IF(WIN32)
  SET(QTCoreModules_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${QTCoreModules_TESTS} ${TESTNAME} 
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SlicerQT
    )
ENDMACRO( SIMPLE_TEST  )

SIMPLE_TEST( qSlicerCamerasModuleTest1 )
SIMPLE_TEST( qSlicerCamerasModuleWidgetTest1 )
ADD_TEST( qSlicerTransformsModuleLogicTest1 ${LAUNCH_EXE}
  ${QTCoreModules_TESTS} qSlicerTransformsModuleLogicTest1
  ${CMAKE_CURRENT_SOURCE_DIR}/affineTransform.txt
  )
SIMPLE_TEST( qSlicerTransformsModuleTest1 )
SIMPLE_TEST( qSlicerTransformsModuleWidgetTest1 )

