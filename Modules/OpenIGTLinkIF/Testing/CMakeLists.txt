set(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out

ADD_TEST(
  OpenIGTLinkIFTestConnector
  ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${OpenIGTLinkIF_SOURCE_DIR}/Testing/TestConnector.tcl
  )
  set_tests_properties( OpenIGTLinkIFTestConnector PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )


