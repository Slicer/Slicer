
IF(Slicer_USE_NUMPY AND PYTHON_EXECUTABLE)
  
  ADD_TEST(SlicerPythonSimpleNUMPYTest ${Slicer_BINARY_DIR}/Slicer3 --launch ${PYTHON_EXECUTABLE} ${Slicer_SOURCE_DIR}/Testing/SimpleNUMPYTest.py)
  
ENDIF()
