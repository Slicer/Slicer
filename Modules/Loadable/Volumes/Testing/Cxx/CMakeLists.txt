set(KIT qSlicer${qt_module_name}Module)

set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${qt_module_name} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

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
target_link_libraries(${KIT}CxxTests ${KIT})

set(VOLUME_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/fixed.nrrd")

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} ${VOLUME_FILE})
endforeach()
