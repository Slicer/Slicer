set(modules
  CropVolumeSequence
  DataProbe
  Editor
  EditorLib
  LabelStatistics
  PerformanceTests
  SampleData
  ScreenCapture
  SegmentEditor
  SegmentStatistics
  SelfTests
  VectorToScalarVolume
  )
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND modules
    DMRIInstall
    ExtensionWizard
    )
endif()
if(Slicer_USE_NUMPY)
  list(APPEND modules
    Endoscopy
    )
endif()
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND modules
    DICOM
    DICOMLib
    DICOMPlugins
    DICOMPatcher
    )
endif()

include(SlicerCheckModuleEnabled)

foreach(module ${modules})
  slicer_is_scripted_builtin_module_enabled(${module} _build_module)
  if(_build_module)
    add_subdirectory(${module})
  endif()
endforeach()
