SET(KIT SceneViews)

SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
#  vtkMRMLSceneViewNodeTest1.cxx
  vtkSlicerSceneViewsModuleLogicTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

MACRO(SIMPLE_TEST  TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME})
ENDMACRO()

#SIMPLE_TEST( vtkMRMLSceneViewNodeTest1 )
#SIMPLE_TEST( vtkSlicerSceneViewsModuleLogicTest1 )

set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

#
# Add all the tests
#
FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TESTNAME ${test} NAME_WE)
  ADD_TEST(
    NAME ${TESTNAME} 
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
      -D ${CMAKE_CURRENT_SOURCE_DIR}/../
      -T ${TEMP}
      #-V Baseline/${TESTNAME}.png
    )
ENDFOREACH()
