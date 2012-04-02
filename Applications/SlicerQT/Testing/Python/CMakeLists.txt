
#
# Check if '--python-code' / 'slicer.testing' works as expected
#

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest1
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "exit(slicer.util.EXIT_SUCCESS)"
  )

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest2
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "exit(slicer.util.EXIT_FAILURE)"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest2 PROPERTIES WILL_FAIL TRUE)

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest3
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --testing --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "import slicer.testing; slicer.testing.exitSuccess()"
  )

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest4
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --testing --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "import slicer.testing; slicer.testing.exitFailure('Should fail !')"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest4 PROPERTIES WILL_FAIL TRUE)

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest5
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
  --ignore-slicerrc --testing --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules --no-main-window
  --python-code "raise Exception('Should fail !')"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest5 PROPERTIES WILL_FAIL TRUE)

#
# Check if '--python-scripts' / 'testing' works as expected
#

slicer_add_python_test(
  SCRIPT SlicerTestingExitSuccessTest.py
  SCRIPT_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_test(
  SCRIPT SlicerTestingExitFailureTest.py
  SCRIPT_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_
  )
set_tests_properties(py_nomainwindow_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)

slicer_add_python_test(SCRIPT SlicerTestingExitSuccessTest.py)

slicer_add_python_test(SCRIPT SlicerTestingExitFailureTest.py)
set_tests_properties(py_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)

#
# Check if 'slicer.testing.runUnitTest()' works as expected
#

slicer_add_python_unittest(
  SCRIPT SlicerUnitTestTest.py
  SLICER_ARGS --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
  TESTNAME_PREFIX nomainwindow_
  )

#
# Exercise different Slicer command line option and check that no warnings are displated.
#
macro(nowarning_test test_name)
  add_test(
    NAME py_nowarning_${test_name}Test
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
    ${ARGN} --testing --python-code "exit(slicer.util.EXIT_SUCCESS)"
    )
  set(fail_regexp)
  list(APPEND fail_regexp "Failed to obtain reference to")
  list(APPEND fail_regexp "Exception AttributeError: .* instance has no attribute")
  set_tests_properties(py_nowarning_${test_name}Test PROPERTIES FAIL_REGULAR_EXPRESSION "${fail_regexp}")
endmacro()
nowarning_test(mainwindow --no-splash)
nowarning_test(mainwindow_python_interactor --show-python-interactor)
nowarning_test(mainwindow_nocli --disable-cli-modules)
nowarning_test(mainwindow_noloadable --disable-loadable-modules)
nowarning_test(mainwindow_noscripted --disable-scripted-loadable-modules)
nowarning_test(mainwindow_nocli_noloadable --disable-cli-modules --disable-loadable-modules)
nowarning_test(mainwindow_nocli_noscripted --disable-cli-modules --disable-scripted-loadable-modules)
nowarning_test(mainwindow_noloadable_noscripted --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(mainwindow_nocli_noloadable_noscripted --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow --no-main-window)
nowarning_test(nomainwindow_python_interactor --no-main-window --show-python-interactor)
nowarning_test(nomainwindow_nocli --no-main-window --disable-cli-modules)
nowarning_test(nomainwindow_noloadable --no-main-window --disable-loadable-modules)
nowarning_test(nomainwindow_noscripted --no-main-window --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_nocli_noloadable --no-main-window --disable-cli-modules --disable-loadable-modules)
nowarning_test(nomainwindow_nocli_noscripted --no-main-window --disable-cli-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_noloadable_noscripted --no-main-window --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_nocli_noloadable_noscripted --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_ignoreslicerrc --no-main-window --ignore-slicerrc)


#
# Application tests
#
slicer_add_python_unittest(
  SCRIPT SlicerSceneObserverTest.py
  SLICER_ARGS --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules
  TESTNAME_PREFIX nomainwindow_
  )
