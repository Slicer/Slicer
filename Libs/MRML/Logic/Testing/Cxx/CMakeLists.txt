set(KIT ${PROJECT_NAME})

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkMRMLAbstractLogicSceneEventsTest.cxx
  vtkMRMLColorLogicTest1.cxx
  vtkMRMLDisplayableHierarchyLogicTest1.cxx
  vtkMRMLLayoutLogicTest1.cxx
  vtkMRMLLayoutLogicTest2.cxx
  vtkMRMLModelHierarchyLogicTest1.cxx
  vtkMRMLSliceLogicTest1.cxx
  vtkMRMLSliceLogicTest2.cxx
  vtkMRMLSliceLogicTest3.cxx
  vtkMRMLSliceLogicTest4.cxx
  EXTRA_INCLUDE TestingMacros.h
)

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})

macro(SIMPLE_FILE_TEST TESTNAME FILE)
  add_test(
    NAME ${TESTNAME}_${FILE}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${MRMLCore_SOURCE_DIR}/Testing/TestData/${FILE}
    )
endmacro()

SIMPLE_TEST( vtkMRMLAbstractLogicSceneEventsTest )
SIMPLE_TEST( vtkMRMLColorLogicTest1 )
SIMPLE_TEST( vtkMRMLDisplayableHierarchyLogicTest1 )
SIMPLE_TEST( vtkMRMLModelHierarchyLogicTest1 )
SIMPLE_TEST( vtkMRMLLayoutLogicTest1 )
SIMPLE_TEST( vtkMRMLLayoutLogicTest2 )
SIMPLE_TEST( vtkMRMLSliceLogicTest1 )
SIMPLE_FILE_TEST( vtkMRMLSliceLogicTest2 fixed.nrrd)
SIMPLE_FILE_TEST( vtkMRMLSliceLogicTest3 fixed.nrrd)
SIMPLE_FILE_TEST( vtkMRMLSliceLogicTest4 fixed.nrrd)
