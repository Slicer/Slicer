
if(Slicer_BUILD_CLI_SUPPORT)
  if(Slicer_BUILD_EMSegment)
    include_directories(
      ${EMSegment_SOURCE_DIR}
      ${RemoteIO_SOURCE_DIR} ${RemoteIO_BINARY_DIR})
  endif()
endif()

set(KIT SlicerQT)
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
set(KIT_TEST_SRCS
  qSlicerAboutDialogTest1.cxx
  qSlicerMainWindowCoreTest1.cxx
  qSlicerMainWindowTest1.cxx
  qSlicerModuleFactoryManagerTest1.cxx
  )
if(Slicer_USE_PYTHONQT_WITH_TCL)
  list(APPEND KIT_TEST_SRCS qSlicerApplicationTpyclTest1.cxx)
  if(Slicer_BUILD_CLI_SUPPORT)
    if(Slicer_BUILD_EMSegment)
      list(APPEND KIT_TEST_SRCS qSlicerApplicationTpyclEMSegmentIntegrationTest.cxx)
    endif()
  endif()
endif()
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  EXTRA_INCLUDE TestingMacros.h
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME qSlicerQT)

set(target_libraries ${LIBRARY_NAME})
if(Slicer_BUILD_CLI_SUPPORT)
  if(Slicer_BUILD_EMSegment)
    list(APPEND target_libraries vtkSlicerEMSegmentModuleLogic)
  endif()
endif()

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests  ${target_libraries})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
endmacro()

#
# Add Tests
#

SIMPLE_TEST( qSlicerAboutDialogTest1 )
if(Slicer_USE_PYTHONQT_WITH_TCL)
  SIMPLE_TEST( qSlicerApplicationTpyclTest1 )
  set_tests_properties(qSlicerApplicationTpyclTest1 PROPERTIES PASS_REGULAR_EXPRESSION stringArray)
  if(Slicer_BUILD_CLI_SUPPORT)
    if(Slicer_BUILD_EMSegment)
      SIMPLE_TEST( qSlicerApplicationTpyclEMSegmentIntegrationTest $<TARGET_FILE:EMSegmentCommandLine>)
    endif()
  endif()
endif()
SIMPLE_TEST( qSlicerMainWindowCoreTest1 )
SIMPLE_TEST( qSlicerMainWindowTest1 )
SIMPLE_TEST( qSlicerModuleFactoryManagerTest1 )

