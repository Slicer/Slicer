SET(KIT BaseLogic)

SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  vtkDataIOManagerLogicTest1.cxx
  vtkSlicerTransformLogicTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
)

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})
ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME})
ENDMACRO()

SIMPLE_TEST( vtkDataIOManagerLogicTest1 )
ADD_TEST(
  NAME vtkSlicerTransformLogicTest1 
  COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> vtkSlicerTransformLogicTest1
    ${CMAKE_CURRENT_SOURCE_DIR}/affineTransform.txt
  )
