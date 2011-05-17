
set(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out

ADD_TEST(
  NeuroNavTestDependency
  ${Slicer_EXECUTABLE} --test-mode --script ${NeuroNav_SOURCE_DIR}/Testing/TestDependency.tcl
  )

set_tests_properties( NeuroNavTestDependency PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )

