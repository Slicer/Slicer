
set(KIT_PYTHON_SCRIPTS
  DICOMScalarVolumePlugin.py
  DICOMDiffusionVolumePlugin.py
  DICOMSlicerDataBundlePlugin.py
  )

set(KIT_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME SlicerDICOMPluginModules
  SCRIPTS "${KIT_PYTHON_SCRIPTS}"
  RESOURCES "${KIT_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
  NO_INSTALL_SUBDIR
  )

if(BUILD_TESTING AND PYTHON_EXECUTABLE)
  # TODO: add testing
  # add_subdirectory(Testing)
endif()
