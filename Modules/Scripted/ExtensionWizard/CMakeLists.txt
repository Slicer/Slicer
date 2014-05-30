#-----------------------------------------------------------------------------
set(MODULE_NAME ExtensionWizard)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  ${MODULE_NAME}Lib/__init__.py
  ${MODULE_NAME}Lib/CreateComponentDialog.py
  ${MODULE_NAME}Lib/DirectoryListWidget.py
  ${MODULE_NAME}Lib/EditableTreeWidget.py
  ${MODULE_NAME}Lib/EditExtensionMetadataDialog.py
  ${MODULE_NAME}Lib/LoadModulesDialog.py
  ${MODULE_NAME}Lib/ModuleInfo.py
  ${MODULE_NAME}Lib/SettingsPanel.py
  ${MODULE_NAME}Lib/TemplatePathUtilities.py
  )

slicerFunctionAddPythonQtResources(MODULE_PYTHON_RESOURCES
  ${MODULE_NAME}Lib/${MODULE_NAME}.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS} ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )
