
set(Slicer_PYTHON_SCRIPTS
  slicer/__init__
  slicer/i18n
  slicer/logic
  slicer/deps/__init__
  slicer/deps/lazy
  slicer/deps/core/__init__
  slicer/parameterNodeWrapper/__init__
  slicer/parameterNodeWrapper/default
  slicer/parameterNodeWrapper/guiCreation
  slicer/parameterNodeWrapper/guiConnectors
  slicer/parameterNodeWrapper/parameterInfo
  slicer/parameterNodeWrapper/parameterPack
  slicer/parameterNodeWrapper/serializers
  slicer/parameterNodeWrapper/types
  slicer/parameterNodeWrapper/util
  slicer/parameterNodeWrapper/validators
  slicer/parameterNodeWrapper/wrapper
  slicer/ScriptedLoadableModule
  slicer/slicerqt
  slicer/testing
  slicer/util
  mrml
  vtkAddon
  vtkITK
  vtkSegmentationCore
  vtkTeem
  )

set(Slicer_PYTHON_MODULES_CONFIG "
# slicer libs
'mrml',
'vtkAddon',
'vtkSegmentationCore',
'qMRMLWidgetsPythonQt',
# slicer base libs
'logic',
'qSlicerBaseQTCorePythonQt',
'qSlicerBaseQTGUIPythonQt',
'qSlicerBaseQTAppPythonQt'"
  )

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND Slicer_PYTHON_SCRIPTS
    slicer/cli
    )
  set(Slicer_PYTHON_MODULES_CONFIG "${Slicer_PYTHON_MODULES_CONFIG},
# CLI logic (Slicer_BUILD_CLI_SUPPORT:ON)
'qSlicerBaseQTCLIPython',
'qSlicerBaseQTCLIPythonQt',
"
  )
endif()

configure_file(
  slicer/kits.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/slicer/kits.py
  @ONLY
  )

list(APPEND Slicer_PYTHON_SCRIPTS ${CMAKE_CURRENT_BINARY_DIR}/slicer/kits.py)

set(Slicer_PYTHON_RESOURCES
  slicer/deps/core/python-extension-manager-requirements-requirements.txt
  slicer/deps/core/python-extension-manager-ssl-requirements-requirements.txt
  slicer/deps/core/python-numpy-requirements.txt
  slicer/deps/core/python-pip-requirements.txt
  slicer/deps/core/python-pythonqt-requirements-requirements.txt
  slicer/deps/core/python-requests-requirements-requirements.txt
  slicer/deps/core/python-scipy-requirements.txt
  slicer/deps/core/python-setuptools-requirements.txt
  slicer/deps/core/python-uv-requirements.txt
  slicer/deps/core/python-wheel-requirements.txt
  )

if(Slicer_USE_SimpleITK)
  list(APPEND Slicer_PYTHON_SCRIPTS
    sitkUtils
    )
endif()

ctkMacroCompilePythonScript(
  TARGET_NAME SlicerBase
  SCRIPTS "${Slicer_PYTHON_SCRIPTS}"
  RESOURCES "${Slicer_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}
  )
