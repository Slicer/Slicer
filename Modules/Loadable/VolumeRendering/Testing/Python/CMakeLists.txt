if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)

  # add tests
  slicer_add_python_unittest(SCRIPT VolumeRenderingSceneClose.py)
  slicer_add_python_unittest(SCRIPT VolumeRenderingThreeDOnlyLayout.py)
  set_tests_properties(
    py_VolumeRenderingThreeDOnlyLayout
    PROPERTIES FAIL_REGULAR_EXPRESSION "OpenGL errors detected"
    )
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
