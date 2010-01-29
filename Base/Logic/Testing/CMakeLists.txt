SET(KIT BaseLogic)

SET( KIT_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

IF(WIN32)
  SET(KIT_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${KIT_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )

SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  vtkDataIOManagerLogicTest1.cxx
  vtkSlicerSliceLogicTest1.cxx
  vtkSlicerTransformLogicTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
)

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})
ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

SIMPLE_TEST( vtkDataIOManagerLogicTest1 )
SIMPLE_TEST( vtkSlicerSliceLogicTest1 )
ADD_TEST( vtkSlicerTransformLogicTest1 ${LAUNCH_EXE}
  ${KIT_TESTS} vtkSlicerTransformLogicTest1
  ${CMAKE_CURRENT_SOURCE_DIR}/affineTransform.txt
  )
