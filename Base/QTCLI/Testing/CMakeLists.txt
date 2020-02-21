
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
set(pycli_build_dir ${SlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
set(pycli_files
  ${CMAKE_CURRENT_SOURCE_DIR}/PyCLIModule4Test.py
  ${CMAKE_CURRENT_SOURCE_DIR}/PyCLIModule4Test.xml
  )

add_custom_target(InstallPyCLITest4 ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${pycli_build_dir}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${pycli_files} ${pycli_build_dir}
  )

if(NOT WIN32)
  add_custom_target(InstallPyCLITest4Chmod ALL
    COMMAND chmod u+x "${pycli_build_dir}/PyCLIModule4Test.py"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS InstallPyCLITest4
  )
endif()

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
