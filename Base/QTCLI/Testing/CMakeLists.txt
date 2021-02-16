
#-----------------------------------------------------------------------------
set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME CLIModule4Test
  FOLDER "Core-Base"
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  NO_INSTALL
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildScriptedCLI(
  NAME PyCLIModule4Test
  FOLDER "Core-Base"
  NO_INSTALL
  )

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qSlicerCLIExecutableModuleFactoryTest1.cxx
  qSlicerCLILoadableModuleFactoryTest1.cxx
  qSlicerCLIModuleTest1.cxx
  )
if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_TEST_SRCS
    qSlicerPyCLIModuleTest1.cxx
    )
endif()

#-----------------------------------------------------------------------------
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})
set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${KIT})
set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Core-Base")

#
# Add Tests
#

simple_test( qSlicerCLIExecutableModuleFactoryTest1 )
simple_test( qSlicerCLILoadableModuleFactoryTest1 )
simple_test( qSlicerCLIModuleTest1 )
if(Slicer_USE_PYTHONQT)
  simple_test( qSlicerPyCLIModuleTest1 )
endif()
