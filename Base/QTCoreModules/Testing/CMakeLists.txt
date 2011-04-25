SET(KIT QTCoreModules)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerCamerasModuleTest1.cxx
  qSlicerCamerasModuleWidgetTest1.cxx
  qSlicerColorsModuleWidgetTest1.cxx
  qSlicerCoreModuleTest1.cxx
  qSlicerCoreModuleFactoryTest1.cxx
  qSlicerModulePanelTest2.cxx
  qSlicerTransformsModuleTest1.cxx
  qSlicerTransformsModuleWidgetTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${PROJECT_NAME})
SET_TARGET_PROPERTIES(${KIT}CxxTests PROPERTIES LABELS ${LIBRARY_NAME})

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(
    NAME ${TESTNAME} 
    COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SlicerQT
    )
  SET_PROPERTY(TEST ${TESTNAME} PROPERTY LABELS ${LIBRARY_NAME})
ENDMACRO()

SIMPLE_TEST( qSlicerCamerasModuleTest1 )
SIMPLE_TEST( qSlicerCamerasModuleWidgetTest1 )
SIMPLE_TEST( qSlicerColorsModuleWidgetTest1 )
SIMPLE_TEST( qSlicerCoreModuleTest1 )
SIMPLE_TEST( qSlicerCoreModuleFactoryTest1 )
SIMPLE_TEST( qSlicerModulePanelTest2 )
SIMPLE_TEST( qSlicerTransformsModuleTest1 )
SIMPLE_TEST( qSlicerTransformsModuleWidgetTest1 )

