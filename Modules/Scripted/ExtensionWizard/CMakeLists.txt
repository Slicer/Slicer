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
  )

set(MODULE_PYTHON_RESOURCES
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )
