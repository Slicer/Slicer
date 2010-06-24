
IF(Slicer3_USE_NUMPY)
  ADD_TEST(Slicer3PythonSimpleNUMPYTest ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Testing/SimpleNUMPYTest.py)
ENDIF(Slicer3_USE_NUMPY)
