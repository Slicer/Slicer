set(KIT FastMarching)
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkPichonFastMarchingTest.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${PROJECT_NAME})

# --------------------------------------------------------------------------
# Testing

set(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out

include_directories(${Slicer_SOURCE_DIR}/Base/Logic)
add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests
  ${lib_name}
  SlicerBaseLogic
  )

set( BaseGUI_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)
set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
set(TEMP_DIR "${Slicer_BINARY_DIR}/Testing/Temporary")

if(WIN32)
  set(BaseGUI_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
endif(WIN32)

add_test(FastMarchingTest1 ${Slicer_LAUNCH_COMMAND} ${KIT}CxxTests vtkPichonFastMarchingTest)
add_test(FastMarchingTest2 ${Slicer_LAUNCH_COMMAND} ${KIT}CxxTests vtkPichonFastMarchingTest
  ${TEST_DATA}/MRMeningioma0.nrrd 132 101 72 10 ${TEMP_DIR}/FastMarchingTest2Output.nrrd)

