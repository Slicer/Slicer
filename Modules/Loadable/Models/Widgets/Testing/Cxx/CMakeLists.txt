set(KIT qSlicer${MODULE_NAME}ModuleWidgets)

#-----------------------------------------------------------------------------
set(INPUT ${MRMLCore_SOURCE_DIR}/Testing)

#-----------------------------------------------------------------------------
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "DEBUG_LEAKS_ENABLE_EXIT_ERROR();")
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  qMRMLModelDisplayNodeWidgetTest1.cxx
  qMRMLModelDisplayNodeWidgetTest2.cxx

  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})

#
# Add Tests
#

simple_test( qMRMLModelDisplayNodeWidgetTest1 )
simple_test( qMRMLModelDisplayNodeWidgetTest2 ${INPUT}/cube.mrml )
