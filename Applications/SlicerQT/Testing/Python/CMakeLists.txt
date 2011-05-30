
#
# Python code
#

ADD_TEST(
  NAME py_nomainwindow_SlicerPythonCodeTest1
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-module --disable-loadable-module --no-main-window
  --python-code "exit(slicer.util.EXIT_SUCCESS)"
  )

ADD_TEST(
  NAME py_nomainwindow_SlicerPythonCodeTest2
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-module --disable-loadable-module --no-main-window
  --python-code "exit(slicer.util.EXIT_FAILURE)"
  )
SET_TESTS_PROPERTIES(py_nomainwindow_SlicerPythonCodeTest2 PROPERTIES WILL_FAIL TRUE)

ADD_TEST(
  NAME py_nomainwindow_SlicerPythonCodeTest3
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-module --disable-loadable-module --no-main-window
  --python-code "import slicer.testing; slicer.testing.setEnabled(); slicer.testing.exitSuccess()"
  )

ADD_TEST(
  NAME py_nomainwindow_SlicerPythonCodeTest4
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-module --disable-loadable-module --no-main-window
  --python-code "import slicer.testing; slicer.testing.setEnabled(); slicer.testing.exitFailure('Should fail !')"
  )
SET_TESTS_PROPERTIES(py_nomainwindow_SlicerPythonCodeTest4 PROPERTIES WILL_FAIL TRUE)

#
# Python scripts
#

SET(TEST_NOMAINWINDOW_SCRIPTS
  SlicerTestingExitSuccessTest.py
  SlicerTestingExitFailureTest.py
  )

SET(TEST_SCRIPTS
  ${TEST_NOMAINWINDOW_SCRIPTS}
  )

MACRO(SLICER_ADD_PYTHON_TEST)
  set(options)
  set(oneValueArgs TESTNAME_PREFIX SLICER_ARGS SCRIPT SCRIPT_ARGS)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  get_filename_component(test_name ${MY_SCRIPT} NAME_WE)
  ADD_TEST(
    NAME py_${MY_TESTNAME_PREFIX}${test_name}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
    --no-splash --ignore-slicerrc ${MY_SLICER_ARGS} --python-script ${CMAKE_CURRENT_SOURCE_DIR}/${MY_SCRIPT} ${MY_SCRIPT_ARGS}
    )
ENDMACRO()

FOREACH(script ${TEST_NOMAINWINDOW_SCRIPTS})
  SLICER_ADD_PYTHON_TEST(SCRIPT ${script} SLICER_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_)
  #SET_TESTS_PROPERTIES(py_${test_name} PROPERTIES RUN_SERIAL TRUE)
ENDFOREACH()

SET_TESTS_PROPERTIES(py_nomainwindow_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)

FOREACH(script ${TEST_SCRIPTS})
  SLICER_ADD_PYTHON_TEST(SCRIPT ${script})
  #SET_TESTS_PROPERTIES(py_${test_name} PROPERTIES RUN_SERIAL TRUE)
ENDFOREACH()

SET_TESTS_PROPERTIES(py_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)

#
# Unittests
#

MACRO(SLICER_ADD_PYTHON_UNITTEST)
  set(options)
  set(oneValueArgs TESTNAME_PREFIX SCRIPT)
  set(multiValueArgs SLICER_ARGS)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  get_filename_component(test_name ${MY_SCRIPT} NAME_WE)
  ADD_TEST(
    NAME py_${MY_TESTNAME_PREFIX}${test_name}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
    --no-splash --ignore-slicerrc ${MY_SLICER_ARGS}
    --python-code "import slicer.testing; slicer.testing.runUnitTest('${CMAKE_CURRENT_SOURCE_DIR}', '${test_name}')"
    )
ENDMACRO()

SLICER_ADD_PYTHON_UNITTEST(
  SCRIPT SlicerUnitTestTest.py
  SLICER_ARGS --no-main-window --disable-cli-module --disable-loadable-module
  TESTNAME_PREFIX nomainwindow_)
