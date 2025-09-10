set(LayerDMManager_PYTHON_SCRIPTS
  __init__.py
  vtkMRMLLayerDMScriptedPipeline.py
)

set(LayerDMManager_PYTHON_RESOURCES
)

ctkMacroCompilePythonScript(
  TARGET_NAME LayerDMManagerLib
  SCRIPTS "${LayerDMManager_PYTHON_SCRIPTS}"
  RESOURCES "${LayerDMManager_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/LayerDMManagerLib
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}/LayerDMManagerLib
  NO_INSTALL_SUBDIR
)
