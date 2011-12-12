
#
# Check if '--python-code' / 'slicer.testing' works as expected
#

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest1
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-module --disable-loadable-module --no-main-window
  --python-code "exit(slicer.util.EXIT_SUCCESS)"
  )

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest2
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-module --disable-loadable-module --no-main-window
  --python-code "exit(slicer.util.EXIT_FAILURE)"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest2 PROPERTIES WILL_FAIL TRUE)

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest3
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-module --disable-loadable-module --no-main-window
  --python-code "import slicer.testing; slicer.testing.setEnabled(); slicer.testing.exitSuccess()"
  )

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest4
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-module --disable-loadable-module --no-main-window
  --python-code "import slicer.testing; slicer.testing.setEnabled(); slicer.testing.exitFailure('Should fail !')"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest4 PROPERTIES WILL_FAIL TRUE)

#
# Check if '--python-scripts' / 'slicer.testing' works as expected
#

slicer_add_python_test(
  SCRIPT SlicerTestingExitSuccessTest.py
  SLICER_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_)

slicer_add_python_test(
  SCRIPT SlicerTestingExitFailureTest.py
  SLICER_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_)
set_tests_properties(py_nomainwindow_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)

slicer_add_python_test(SCRIPT SlicerTestingExitSuccessTest.py)

slicer_add_python_test(SCRIPT SlicerTestingExitFailureTest.py)
set_tests_properties(py_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)

#
# Check if 'slicer.testing.runUnitTest()' works as expected
#

slicer_add_python_unittest(
  SCRIPT SlicerUnitTestTest.py
  SLICER_ARGS --no-main-window --disable-cli-module --disable-loadable-module
  TESTNAME_PREFIX nomainwindow_)
