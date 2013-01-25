
set(SCRIPTED_MODULES
  DataProbe
  Editor
  FiberBundleToLabelMap
  LabelStatistics
  PerformanceTests
  SampleData
  VectorToScalarVolume
  SelfTests
  )
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND SCRIPTED_MODULES
    DICOM
    )
endif()
if(Slicer_USE_NUMPY)
  list(APPEND SCRIPTED_MODULES
    Endoscopy
    )
endif()

foreach(module ${SCRIPTED_MODULES})
  list(FIND Slicer_QTSCRIPTEDMODULES_DISABLED ${module} _module_disabled)
  if(${_module_disabled} STREQUAL "-1")
    slicerMacroBuildScriptedModule(
      NAME "${module}"
      SCRIPTS "${module}.py"
      )
  endif()
endforeach()

if(Slicer_BUILD_DICOM_SUPPORT)
  if(Slicer_BUILD_DIFFUSION_SUPPORT)
    add_subdirectory(
      DICOM2FullBrainTractography
      )
  endif()

  add_subdirectory(
    DICOMPlugins
    )
endif()

if(BUILD_TESTING AND PYTHON_EXECUTABLE)

  # this will regiester the unittest subclass in the main script
  # as a ctest (note that it is also usable as a runtime test).
  slicer_add_python_unittest(SCRIPT FiberBundleToLabelMap.py)

  add_subdirectory(Testing)
endif()
