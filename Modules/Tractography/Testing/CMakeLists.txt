
set(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out

ADD_TEST(
  TractographyTestGlyphs
  ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Tractography_SOURCE_DIR}/Testing/TestGlyphs.tcl
  )
  set_tests_properties( TractographyTestGlyphs PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )

