# --------------------------------------------------------------------------
# Testing
ADD_TEST(Slicer3PythonTest ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Python/Testing/SimpleTest.py)

IF(Slicer3_USE_NUMPY)
  ADD_TEST(Slicer3PythonSimpleNUMPYTest ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Base/GUI/Python/Testing/SimpleNUMPYTest.py)
ENDIF(Slicer3_USE_NUMPY)
