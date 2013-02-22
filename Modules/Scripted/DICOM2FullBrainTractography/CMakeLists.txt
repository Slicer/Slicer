
#-----------------------------------------------------------------------------
set(MODULE_NAME DICOM2FullBrainTractography)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  ${MODULE_NAME}Lib/__init__
  ${MODULE_NAME}Lib/full_tractography_workflow
  ${MODULE_NAME}Lib/workflow_support
  )

set(MODULE_PYTHON_RESOURCES
  ${MODULE_NAME}Lib/Resources/UI/dicom2nrrd.ui
  ${MODULE_NAME}Lib/Resources/UI/done.ui
  ${MODULE_NAME}Lib/Resources/UI/dti2fibers.ui
  ${MODULE_NAME}Lib/Resources/UI/dwi2dti.ui
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )
