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

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})
SET_TARGET_PROPERTIES(${KIT}CxxTests PROPERTIES LABELS ${LIBRARY_NAME})

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME})
  SET_PROPERTY(TEST ${TESTNAME} PROPERTY LABELS ${LIBRARY_NAME})
ENDMACRO()

#
# Add Tests
#

# Remark: qSlicerModuleFactoryManager class is tested within Applications/SlicerQT/Testing

SET(testname qSlicerCoreApplicationTest1)
ADD_TEST(
  NAME ${testname}
  COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> qSlicerCoreApplicationTest1
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SlicerQT )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${LIBRARY_NAME})

SET(testname qSlicerCoreIOManagerTest1)
ADD_TEST(
  NAME ${testname}
  COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> qSlicerCoreIOManagerTest1
    ${CMAKE_SOURCE_DIR}/Libs/MRML/Testing/name.mrml
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${LIBRARY_NAME})

SIMPLE_TEST( qSlicerAbstractCoreModuleTest1 )
SIMPLE_TEST( qSlicerLoadableModuleFactoryTest1 )
SIMPLE_TEST( qSlicerUtilsTest1 )
