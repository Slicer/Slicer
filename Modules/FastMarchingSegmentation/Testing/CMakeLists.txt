SET(KIT FastMarching)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  vtkPichonFastMarchingTest.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

# --------------------------------------------------------------------------
# Testing

SET(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out

INCLUDE_DIRECTORIES(${Slicer_SOURCE_DIR}/Base/Logic)
ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests 
  ${lib_name} 
  SlicerBaseLogic
  )

SET( BaseGUI_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)
SET(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
SET(TEMP_DIR "${Slicer_BINARY_DIR}/Testing/Temporary")

IF(WIN32)
  SET(BaseGUI_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

ADD_TEST(FastMarchingTest1 ${Slicer_LAUNCH_COMMAND} ${KIT}CxxTests vtkPichonFastMarchingTest)
ADD_TEST(FastMarchingTest2 ${Slicer_LAUNCH_COMMAND} ${KIT}CxxTests vtkPichonFastMarchingTest
  ${TEST_DATA}/MRMeningioma0.nrrd 132 101 72 10 ${TEMP_DIR}/FastMarchingTest2Output.nrrd)

