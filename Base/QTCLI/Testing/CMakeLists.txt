
slicerMacroBuildCLI(
  NAME CLIModule4Test
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  NO_INSTALL
  )

set(KIT QTCLI)
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  qSlicerCLIExecutableModuleFactoryTest1.cxx
  qSlicerCLILoadableModuleFactoryTest1.cxx
  qSlicerCLIModuleTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${PROJECT_NAME})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})
set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${LIBRARY_NAME})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
  set_property(TEST ${TESTNAME} PROPERTY LABELS ${LIBRARY_NAME})
endmacro()

#
# Add Tests
#

SIMPLE_TEST( qSlicerCLIExecutableModuleFactoryTest1 )
SIMPLE_TEST( qSlicerCLILoadableModuleFactoryTest1 )
SIMPLE_TEST( qSlicerCLIModuleTest1 )
