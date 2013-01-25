
set(MODULE_PYTHON_SCRIPTS
  DICOMScalarVolumePlugin.py
  DICOMDiffusionVolumePlugin.py
  DICOMSlicerDataBundlePlugin.py
  )

set(MODULE_PYTHON_RESOURCES
  )

slicerMacroBuildScriptedModule(
  NAME "SlicerDICOMPluginModules"
  SCRIPTS "${MODULE_PYTHON_SCRIPTS}"
  RESOURCES "${MODULE_PYTHON_RESOURCES}"
  )

if(BUILD_TESTING AND PYTHON_EXECUTABLE)
  # TODO: add testing
  # add_subdirectory(Testing)
endif()
