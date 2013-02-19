set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(TEMP ${Slicer_BINARY_DIR}/Testing/Temporary)
set(INPUT ${MRMLCore_SOURCE_DIR}/Testing/TestData/)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${MODULE_NAME} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

#-----------------------------------------------------------------------------
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  qSlicerVolumesIOOptionsWidgetTest1.cxx
  qSlicerVolumesModuleWidgetTest1.cxx
  vtkSlicerVolumesLogicTest1.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

add_executable(${KIT}CxxTests ${Tests})
set_target_properties(${KIT}CxxTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR})
target_link_libraries(${KIT}CxxTests ${KIT})

#-----------------------------------------------------------------------------
foreach(testname ${KIT_TEST_NAMES})
  simple_test( ${testname})
endforeach()
simple_test( qSlicerVolumesIOOptionsWidgetTest1 )
simple_test( qSlicerVolumesModuleWidgetTest1 ${INPUT}/fixed.nrrd)
simple_test( vtkSlicerVolumesLogicTest1 ${INPUT}/fixed.nrrd)
  
#-----------------------------------------------------------------------------
add_executable(vtkSlicerVolumesLogicTest2 vtkSlicerVolumesLogicTest2.cxx)
target_link_libraries(vtkSlicerVolumesLogicTest2 ${KIT})

add_test(NAME vtkSlicerVolumesLogicTest2 
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:vtkSlicerVolumesLogicTest2>
  vtkSlicerVolumesLogicTest2 ${TEMP}
  )
