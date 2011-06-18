SET(KIT QTCore)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qSlicerAbstractCoreModuleTest1
  qSlicerCoreApplicationTest1.cxx
  qSlicerCoreIOManagerTest1.cxx
  qSlicerLoadableModuleFactoryTest1.cxx
  qSlicerUtilsTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET(TestsToRun ${Tests})
LIST(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})
SET_TARGET_PROPERTIES(${KIT}CxxTests PROPERTIES LABELS ${LIBRARY_NAME})

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
  SET_PROPERTY(TEST ${TESTNAME} PROPERTY LABELS ${LIBRARY_NAME})
ENDMACRO()

#
# Add Tests
#

# Remark: qSlicerModuleFactoryManager class is tested within Applications/SlicerQT/Testing

SIMPLE_TEST( qSlicerCoreApplicationTest1)
SET_PROPERTY(TEST qSlicerCoreApplicationTest1 PROPERTY LABELS ${LIBRARY_NAME})
SIMPLE_TEST( qSlicerCoreIOManagerTest1 ${CMAKE_SOURCE_DIR}/Libs/MRML/Testing/name.mrml)
SET_PROPERTY(TEST qSlicerCoreIOManagerTest1 PROPERTY LABELS ${LIBRARY_NAME})
SIMPLE_TEST( qSlicerAbstractCoreModuleTest1 )
SIMPLE_TEST( qSlicerLoadableModuleFactoryTest1 )
SIMPLE_TEST( qSlicerUtilsTest1 )
