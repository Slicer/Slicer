
SET(KIT_PYTHON_SCRIPTS
    Editor.py
    LabelStatistics.py
    SampleData.py
    DICOM.py
  )

IF(Slicer_USE_NUMPY)
  LIST(APPEND KIT_PYTHON_SCRIPTS
    Endoscopy.py
    )
ENDIF()
  
SET(KIT_PYTHON_RESOURCES
  )
  
ctkMacroCompilePythonScript(
  TARGET_NAME SlicerQTScriptedModules
  SCRIPTS "${KIT_PYTHON_SCRIPTS}"
  RESOURCES "${KIT_PYTHON_RESOURCES}"
  DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR .
  )
