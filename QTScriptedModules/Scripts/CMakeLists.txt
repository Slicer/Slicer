
set(KIT_PYTHON_SCRIPTS
    DataProbe.py
    DICOM.py
    Editor.py
    LabelStatistics.py
    PerformanceTests.py
    SampleData.py
  )

if(Slicer_USE_NUMPY)
  list(APPEND KIT_PYTHON_SCRIPTS
    Endoscopy.py
    )
endif()

set(KIT_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME SlicerQTScriptedModules
  SCRIPTS "${KIT_PYTHON_SCRIPTS}"
  RESOURCES "${KIT_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
  NO_INSTALL_SUBDIR
  )

add_subdirectory(
  DICOM2FullBrainTractography
  )
