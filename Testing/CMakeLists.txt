
if(Slicer_USE_NUMPY AND Python3_EXECUTABLE)

  add_test(PythonSlicerSimpleNUMPYTest ${Slicer_LAUNCH_COMMAND} ${Python3_EXECUTABLE} ${Slicer_SOURCE_DIR}/Testing/SimpleNUMPYTest.py)

endif()
