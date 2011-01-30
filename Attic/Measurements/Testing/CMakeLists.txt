SET(KIT MeasurementsLib)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  vtkMRMLMeasurementsNodeTest1.cxx
  vtkMRMLMeasurementsRulerNodeTest1.cxx
  vtkMRMLMeasurementsAngleNodeTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )
SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

SET( ${KIT}_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

IF(WIN32)
  SET(${KIT}_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${${KIT}_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )

SIMPLE_TEST( vtkMRMLMeasurementsNodeTest1 )
SIMPLE_TEST( vtkMRMLMeasurementsRulerNodeTest1 )
SIMPLE_TEST( vtkMRMLMeasurementsAngleNodeTest1 )

# add a tcl test 
# ADD_TEST(MeasurementsTest1 ${Slicer_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer_SOURCE_DIR}/Modules/Measurements/Testing/Tcl/MeasurementsTest1.tcl)
