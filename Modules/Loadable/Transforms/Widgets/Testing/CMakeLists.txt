
set(KIT ${PROJECT_NAME})
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "DEBUG_LEAKS_ENABLE_EXIT_ERROR();")
set(TEST_SOURCES
  qMRMLTransformInfoWidgetTest1.cxx
  qMRMLTransformDisplayNodeWidgetTest1.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${TEST_SOURCES}
  )

include_directories( ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${KIT}CxxTests ${Tests} )
target_link_libraries(${KIT}CxxTests ${KIT} )
set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Module-${MODULE_NAME}")

macro(SCENE_TEST TESTNAME SCENEFILENAME)
  add_test(
    NAME ${TESTNAME}_${SCENEFILENAME}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${MRMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME}
    )
endmacro()

simple_test( qMRMLTransformInfoWidgetTest1 )
simple_test( qMRMLTransformDisplayNodeWidgetTest1 )
