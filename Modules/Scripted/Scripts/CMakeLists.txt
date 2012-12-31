
set(KIT_PYTHON_SCRIPTS
    DataProbe.py
    Editor.py
    LabelStatistics.py
    PerformanceTests.py
    SampleData.py
    VectorToScalarVolume.py
    SelfTests.py
  )

if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND KIT_PYTHON_SCRIPTS
    DICOM.py
    )
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  list(APPEND KIT_PYTHON_SCRIPTS
    Editor.py
    )
endif()

if(Slicer_USE_NUMPY)
  list(APPEND KIT_PYTHON_SCRIPTS
    Endoscopy.py
    )
endif()

set(KIT_PYTHON_RESOURCES
  )

set(MODULE_SCRIPTS
  )
foreach(module ${KIT_PYTHON_SCRIPTS})
  list(FIND Slicer_QTSCRIPTEDMODULES_DISABLED ${module} _module_disabled)
  if(${_module_disabled} STREQUAL "-1")
    list(APPEND MODULE_SCRIPTS ${module})
  endif()
endforeach()

ctkMacroCompilePythonScript(
  TARGET_NAME SlicerQTScriptedModules
  SCRIPTS "${MODULE_SCRIPTS}"
  RESOURCES "${KIT_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
  NO_INSTALL_SUBDIR
  )

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
  add_subdirectory(Testing)
endif()
