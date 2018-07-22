
if(Slicer_USE_NUMPY AND PYTHON_EXECUTABLE)

  add_test(PythonSlicerSimpleNUMPYTest ${Slicer_LAUNCH_COMMAND} ${PYTHON_EXECUTABLE} ${Slicer_SOURCE_DIR}/Testing/SimpleNUMPYTest.py)

endif()
