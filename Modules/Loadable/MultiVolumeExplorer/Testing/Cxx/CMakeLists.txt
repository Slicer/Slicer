set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${EXTENSION_NAME} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

#set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${EXTENSION_NAME}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  # Add source of your tests after this line.
  #EXTRA_INCLUDE TestingMacros.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

#set(TestsToRun ${Tests})
#list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME qSlicer${EXTENSION_NAME}Module)

add_executable(${EXTENSION_NAME}CxxTests ${Tests})
target_link_libraries(${EXTENSION_NAME}CxxTests ${lib_name})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${EXTENSION_NAME}CxxTests> ${TESTNAME} ${ARGN})
endmacro()

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
endforeach()

# Using SIMPLE_TEST(), you coudl add your test after this line.
