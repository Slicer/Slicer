
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
    --no-splash ${MY_SLICER_ARGS} --python-script ${CMAKE_CURRENT_SOURCE_DIR}/${MY_SCRIPT} ${MY_SCRIPT_ARGS}
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

