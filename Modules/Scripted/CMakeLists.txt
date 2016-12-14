
include(SlicerMacroBuildScriptedModule)

set(modules
  DataProbe
  DMRIInstall
  Editor
  EditorLib
  LabelStatistics
  PerformanceTests
  SampleData
  ScreenCapture
  SegmentEditor
  SegmentStatistics
  SelfTests
  SurfaceToolbox
  VectorToScalarVolume
  )
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND modules
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
foreach(module ${modules})
  list(FIND Slicer_QTSCRIPTEDMODULES_DISABLED ${module} _module_disabled)
  if(${_module_disabled} STREQUAL "-1")
    add_subdirectory(${module})
  endif()
endforeach()

# External modules have been moved to Modules/Remote/CMakeLists.txt
