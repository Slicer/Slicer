set(KIT ${PROJECT_NAME})

slicerMacroBuildCLI(
  NAME CLIModule4Test
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  NO_INSTALL
  )

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  qSlicerCLIExecutableModuleFactoryTest1.cxx
  qSlicerCLILoadableModuleFactoryTest1.cxx
  qSlicerCLIModuleTest1.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})
set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${KIT})

#
# Add Tests
#

SIMPLE_TEST( qSlicerCLIExecutableModuleFactoryTest1 )
SIMPLE_TEST( qSlicerCLILoadableModuleFactoryTest1 )
SIMPLE_TEST( qSlicerCLIModuleTest1 )
