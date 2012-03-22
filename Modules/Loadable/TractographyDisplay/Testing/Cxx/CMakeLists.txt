set(KIT qSlicer${qt_module_name}Module)

set(KIT_TEST_SRCS qSlicerTractographyDisplayGlyphWidgetTest1.cxx)
set(KIT_TEST_NAMES qSlicerTractographyDisplayGlyphWidgetTest1)
set(KIT_TEST_NAMES_CXX qSlicerTractographyDisplayGlyphWidgetTest1.cxx)
SlicerMacroConfigureGenericCxxModuleTests(${qt_module_name} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

set(LIBRARY_NAME ${lib_name})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
endforeach()

