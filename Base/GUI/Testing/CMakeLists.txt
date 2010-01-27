# --------------------------------------------------------------------------
# Testing

SET(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out

ADD_TEST(Slicer3GUITest1 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestSliceController.tcl)
ADD_TEST(Slicer3GUITest2 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestProgressDialog.tcl)
ADD_TEST(Slicer3FiducialsTest1 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestFiducials.tcl)
ADD_TEST(Slicer3FiducialsTest2 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestFiducialsAdd.tcl)
ADD_TEST(Slicer3FiducialsTestGlyphs ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestFiducialsGlyphs.tcl)

SET_TESTS_PROPERTIES( Slicer3FiducialsTest1 PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )
SET_TESTS_PROPERTIES( Slicer3FiducialsTest2 PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )
