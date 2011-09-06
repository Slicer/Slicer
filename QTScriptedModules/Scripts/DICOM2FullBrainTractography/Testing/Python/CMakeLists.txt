set(KIT_UNITTEST_SCRIPTS)
SlicerMacroConfigureGenericPythonModuleTests("${EXTENSION_NAME}" KIT_UNITTEST_SCRIPTS)

foreach(script_name ${KIT_UNITTEST_SCRIPTS})
  SLICER_ADD_PYTHON_UNITTEST(
    SCRIPT ${script_name}
    SLICER_ARGS --no-main-window --disable-cli-module TESTNAME_PREFIX nomainwindow_)
endforeach()

