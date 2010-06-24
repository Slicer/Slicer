
IF(Slicer3_USE_NUMPY)
  ADD_TEST(Slicer3PythonSimpleNUMPYTest ${PYTHON_EXECUTABLE} ${Slicer3_SOURCE_DIR}/Testing/SimpleNUMPYTest.py)
ENDIF(Slicer3_USE_NUMPY)
