SET(KIT QTCoreModules)

SET(KIT_TEST_SRCS)
SET(KIT_TEST_NAMES)
SET(KIT_TEST_NAMES_CXX)
SET(modulenames Cameras Colors Data EventBroker ROI SliceController Transforms)
SlicerMacroAddGenericCxxModuleTests("${modulenames}" KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  qSlicerCamerasModuleWidgetTest1.cxx
  qSlicerColorsModuleWidgetTest1.cxx
  qSlicerCoreModuleTest1.cxx
  qSlicerCoreModuleFactoryTest1.cxx
  qSlicerModulePanelTest2.cxx
  qSlicerTransformsModuleTest1.cxx
  qSlicerTransformsModuleWidgetTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

LIST(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
LIST(APPEND Tests ${KIT_TEST_SRCS})

#SET(TestsToRun ${Tests})
#LIST(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${PROJECT_NAME})
SET_TARGET_PROPERTIES(${KIT}CxxTests PROPERTIES LABELS ${LIBRARY_NAME})

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN}
    )
  SET_PROPERTY(TEST ${TESTNAME} PROPERTY LABELS ${LIBRARY_NAME})
ENDMACRO()

FOREACH(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
ENDFOREACH()
SIMPLE_TEST( qSlicerCamerasModuleWidgetTest1 )
SIMPLE_TEST( qSlicerColorsModuleWidgetTest1 )
SIMPLE_TEST( qSlicerCoreModuleTest1 )
SIMPLE_TEST( qSlicerCoreModuleFactoryTest1 )
SIMPLE_TEST( qSlicerModulePanelTest2 )
SIMPLE_TEST( qSlicerTransformsModuleWidgetTest1 )

