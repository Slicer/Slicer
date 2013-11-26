if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)

  # add a self test
  slicer_add_python_unittest(SCRIPT VolumeRenderingSceneClose.py)
  set(KIT_PYTHON_SCRIPTS
    VolumeRenderingSceneClose.py
     )
  ctkMacroCompilePythonScript(
    TARGET_NAME ApplicationSelfTests
    SCRIPTS "${KIT_PYTHON_SCRIPTS}"
    DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
    INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
    NO_INSTALL_SUBDIR
    )

endif()
