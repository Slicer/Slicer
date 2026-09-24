
#-----------------------------------------------------------------------------
set(MODULE_NAME DICOMPlugins)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  DICOMEnhancedUSVolumePlugin.py
  DICOMGeAbusPlugin.py
  DICOMImageSequencePlugin.py
  DICOMScalarVolumePlugin.py
  DICOMVolumeSequencePlugin.py
  DICOMSlicerDataBundlePlugin.py
  DICOMSegmentationPlugin.py
  DICOMParametricMapPlugin.py
  DICOMTID1500Plugin.py
  DICOMM3DPlugin.py
  )
if(Slicer_BUILD_MULTIVOLUME_SUPPORT)
  list(APPEND MODULE_PYTHON_SCRIPTS
    MultiVolumeImporterPlugin.py
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
