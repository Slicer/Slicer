set(KIT qSlicer${EXTENSION_NAME}Module)

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

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
endforeach()

# Using SIMPLE_TEST(), you could add your test after this line.
