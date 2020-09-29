
#-----------------------------------------------------------------------------
set(MODULE_NAME DICOMPlugins)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  DICOMImageSequencePlugin.py
  DICOMScalarVolumePlugin.py
  DICOMVolumeSequencePlugin.py
  DICOMSlicerDataBundlePlugin.py
  )

set(MODULE_PYTHON_RESOURCES
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  )
