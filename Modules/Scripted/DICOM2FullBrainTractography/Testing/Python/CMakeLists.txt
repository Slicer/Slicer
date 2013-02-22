set(KIT_UNITTEST_SCRIPTS)
SlicerMacroConfigureGenericPythonModuleTests("${MODULE_NAME}" KIT_UNITTEST_SCRIPTS)

foreach(script_name ${KIT_UNITTEST_SCRIPTS})
  slicer_add_python_unittest(
    SCRIPT ${script_name}
    SLICER_ARGS --no-main-window --disable-cli-module TESTNAME_PREFIX nomainwindow_)
endforeach()

