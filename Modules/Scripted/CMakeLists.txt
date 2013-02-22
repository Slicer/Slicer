
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

if(Slicer_BUILD_MultiVolumeImporter)
  # NOTE: MultiVolumeImporter source code is checkout using "External_MultiVolumeImporter.cmake".
  add_subdirectory(
    ${MultiVolumeImporter_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/MultiVolumeImporter
    )
endif()
