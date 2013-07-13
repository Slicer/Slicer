
include(SlicerMacroBuildScriptedModule)

set(modules
  DataProbe
  Editor
  EditorLib
  FiberBundleToLabelMap
  LabelStatistics
  PerformanceTests
  SampleData
  SelfTests
  VectorToScalarVolume
  )
if(Slicer_USE_NUMPY)
  list(APPEND modules
    Endoscopy
    )
endif()
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND modules
    DICOM
    DICOMPlugins
    )
  if(Slicer_BUILD_DIFFUSION_SUPPORT)
    list(APPEND modules
      DICOM2FullBrainTractography
      )
  endif()
endif()

foreach(module ${modules})
  list(FIND Slicer_QTSCRIPTEDMODULES_DISABLED ${module} _module_disabled)
  if(${_module_disabled} STREQUAL "-1")
    add_subdirectory(${module})
  endif()
endforeach()

# External modules have been moved to Modules/Remote/CMakeLists.txt
