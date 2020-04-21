set(KIT ${APPLIB_NAME})

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
set(KIT_TEST_SRCS
  qSlicerAppMainWindowTest1.cxx
  qSlicerModuleFactoryManagerTest1.cxx
  )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

set(target_libraries ${KIT})

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests  ${target_libraries})
set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

#
# Add Tests
#
simple_test( qSlicerAppMainWindowTest1 )
simple_test( qSlicerModuleFactoryManagerTest1 )

#
# Application tests
#

if(Slicer_HAS_CONSOLE_IO_SUPPORT)
  add_test(
    NAME slicer_nomainwindow_DisableModulesCommandLineOptionsTest
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
    ${ARGN} --testing --verbose-module-discovery --disable-modules
    )
  add_test(
    NAME slicer_nomainwindow_DisableModulesCommandLineOptionsTest2
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
    ${ARGN} --testing --verbose-module-discovery --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
    )
  set_tests_properties(
    slicer_nomainwindow_DisableModulesCommandLineOptionsTest
    slicer_nomainwindow_DisableModulesCommandLineOptionsTest2
    PROPERTIES PASS_REGULAR_EXPRESSION "Number of loaded modules: 1"
    )

  add_test(
    NAME slicer_nomainwindow_NoApplicationInformationOptionTest
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
    --disable-modules --ignore-slicerrc --exit-after-startup
    )
  set_tests_properties(
    slicer_nomainwindow_NoApplicationInformationOptionTest
    PROPERTIES FAIL_REGULAR_EXPRESSION "Session start time"
    )
  add_test(
    NAME slicer_nomainwindow_ApplicationInformationOptionTest
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
    --disable-modules --ignore-slicerrc --exit-after-startup --application-information
    )
  set_tests_properties(
    slicer_nomainwindow_ApplicationInformationOptionTest
    PROPERTIES PASS_REGULAR_EXPRESSION "Session start time"
    )
endif()

