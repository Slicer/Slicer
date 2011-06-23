set(KIT ModelTransformLib)
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkMRMLModelTransformNodeTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )
#set(TestsToRun ${Tests})
#list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${PROJECT_NAME})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${lib_name})

set(${KIT}_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

if(WIN32)
  set(${KIT}_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
endif(WIN32)

macro(SIMPLE_TEST TESTNAME)
  add_test(${TESTNAME} ${Slicer_LAUNCH_COMMAND} ${${KIT}_TESTS} ${TESTNAME} ${ARGN})
endmacro()

SIMPLE_TEST( vtkMRMLModelTransformNodeTest1 )
