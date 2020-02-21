set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(${KIT}Testing_ITK_COMPONENTS
  ITKCommon
  )
find_package(ITK 4.6 COMPONENTS ${${KIT}Testing_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS
  ${ITKFactoryRegistration_INCLUDE_DIRS}
  )
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
set(EXTRA_INCLUDE "vtkMRMLDebugLeaksMacro.h\"\n\#include \"vtkTestingOutputWindow.h")
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();\nTESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);" )
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkMRMLAbstractLogicSceneEventsTest.cxx
  vtkMRMLColorLogicTest1.cxx
  vtkMRMLDisplayableHierarchyLogicTest1.cxx
  vtkMRMLLayoutLogicCompareTest.cxx
  vtkMRMLLayoutLogicTest1.cxx
  vtkMRMLLayoutLogicTest2.cxx
  vtkMRMLSliceLayerLogicTest.cxx
  vtkMRMLSliceLogicTest1.cxx
  vtkMRMLSliceLogicTest2.cxx
  vtkMRMLSliceLogicTest3.cxx
  vtkMRMLSliceLogicTest4.cxx
  vtkMRMLSliceLogicTest5.cxx
  vtkMRMLApplicationLogicTest1.cxx
  EXTRA_INCLUDE ${EXTRA_INCLUDE}
  )

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT} ${ITK_LIBRARIES})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

#-----------------------------------------------------------------------------
macro(simple_file_test TESTNAME FILE)
  simple_test(${TESTNAME}_${FILE} DRIVER_TESTNAME ${TESTNAME} DATA{${MRMLCore_SOURCE_DIR}/Testing/TestData/${FILE}})
endmacro()

#-----------------------------------------------------------------------------
simple_test( vtkMRMLAbstractLogicSceneEventsTest )
simple_test( vtkMRMLColorLogicTest1 )
simple_test( vtkMRMLDisplayableHierarchyLogicTest1 )
simple_test( vtkMRMLLayoutLogicCompareTest )
simple_test( vtkMRMLLayoutLogicTest1 )
simple_test( vtkMRMLLayoutLogicTest2 )
simple_test( vtkMRMLSliceLayerLogicTest )
simple_test( vtkMRMLSliceLogicTest1 )
simple_file_test( vtkMRMLSliceLogicTest2 fixed.nrrd)
simple_file_test( vtkMRMLSliceLogicTest3 fixed.nrrd)
simple_file_test( vtkMRMLSliceLogicTest4 fixed.nrrd)
simple_file_test( vtkMRMLSliceLogicTest5 fixed.nrrd)
simple_test( vtkMRMLApplicationLogicTest1 "${CMAKE_BINARY_DIR}/Testing/Temporary" )
