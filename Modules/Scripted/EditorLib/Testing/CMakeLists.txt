
slicer_add_python_unittest(SCRIPT ThresholdThreadingTest.py)
slicer_add_python_unittest(SCRIPT StandaloneEditorWidgetTest.py)


set(KIT_PYTHON_SCRIPTS
  ThresholdThreadingTest.py
  )

set(KIT_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME EditorLibTests
  SCRIPTS "${KIT_PYTHON_SCRIPTS}"
  RESOURCES "${KIT_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
  NO_INSTALL_SUBDIR
  )
