
#-----------------------------------------------------------------------------
set(MODULE_NAME DICOMPlugins)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  DICOMScalarVolumePlugin.py
  DICOMSlicerDataBundlePlugin.py
  )

if(Slicer_BUILD_DIFFUSION_SUPPORT)
  list(APPEND MODULE_PYTHON_SCRIPTS
    DICOMDiffusionVolumePlugin.py
    )
endif()

set(MODULE_PYTHON_RESOURCES
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  )
