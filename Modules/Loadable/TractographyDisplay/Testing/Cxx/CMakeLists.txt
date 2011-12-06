set(KIT ${qt_module_name})

set(KIT_TEST_SRCS qSlicerTractographyDisplayGlyphWidgetTest1.cxx)
set(KIT_TEST_NAMES qSlicerTractographyDisplayGlyphWidgetTest1)
set(KIT_TEST_NAMES_CXX qSlicerTractographyDisplayGlyphWidgetTest1.cxx)
SlicerMacroConfigureGenericCxxModuleTests(${KIT} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  EXTRA_INCLUDE TestingMacros.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

#set(TestsToRun ${Tests})
#list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${lib_name})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME})
endmacro()

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
endforeach()

