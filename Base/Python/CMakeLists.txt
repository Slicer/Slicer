
set(Slicer_PYTHON_SCRIPTS
  slicer/__init__
  slicer/logic
  slicer/ScriptedLoadableModule
  slicer/slicerqt
  slicer/testing
  slicer/util
  slicer/moduledm/__init__
  slicer/modulelogic/__init__
  slicer/modulemrml/__init__
  slicer/modulewidget/__init__
  freesurfer
  mrml
  mrmlLogic
  saferef
  teem
  vtkAddon
  vtkITK
  )

set(Slicer_PYTHON_MODULES_CONFIG "
# slicer libs
'freesurfer',
'mrml',
'mrmlLogic',
'teem',
'qMRMLWidgetsPythonQt',
# slicer base libs
'logic',
'qSlicerBaseQTCorePythonQt',
'qSlicerBaseQTGUIPythonQt',
'qSlicerBaseQTAppPythonQt',
# slicer module
'moduledm',
'modulelogic',
'modulemrml',
'modulewidget'"
  )

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND Slicer_PYTHON_SCRIPTS
    slicer/cli
    mrmlcli
    )
  set(Slicer_PYTHON_MODULES_CONFIG "${Slicer_PYTHON_MODULES_CONFIG},
# CLI logic (Slicer_BUILD_CLI_SUPPORT:ON)
'mrmlcli',
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
  )

if(Slicer_USE_SimpleITK)
  list(APPEND Slicer_PYTHON_SCRIPTS
    sitkUtils
    )
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  list(APPEND Slicer_PYTHON_SCRIPTS
    slicer/slicerqt-with-tcl
    tpycl/__init__
    tpycl/tpycl
    )
  list(APPEND Slicer_PYTHON_RESOURCES
    tpycl/tpycl.tcl
    slicer/tcl/Slicer3Adapters.tcl
    )
endif()

ctkMacroCompilePythonScript(
  TARGET_NAME SlicerBase
  SCRIPTS "${Slicer_PYTHON_SCRIPTS}"
  RESOURCES "${Slicer_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python
  INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}
  )

#-----------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
  if(Slicer_USE_PYTHONQT_WITH_TCL)
    add_subdirectory(tpycl/Testing)
  endif()
endif()
