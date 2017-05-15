if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT AND Slicer_USE_PYTHONQT)
  # Determine name of Python interpreter
  #
  # NOTE: Do NOT use ${PYTHON_EXECUTABLE}; when using our own Python, it is set
  #       to a ctkAppLauncher wrapper, and we don't want that!
  if(Slicer_USE_SYSTEM_python)
    set(PYTHON python)
  else()
    find_program(PYTHON python
      PATHS ${Slicer_SUPERBUILD_DIR}/python-install/bin
      NO_DEFAULT_PATH
      )
  endif()

  # Configure and Extension Wizard launcher scripts
  configure_file(
    slicerExtensionWizard.sh.in
    ${Slicer_BINARY_DIR}/bin/slicerExtensionWizard @ONLY
    )

  configure_file(
    slicerExtensionWizard-install.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/slicerExtensionWizard-install.sh @ONLY
    )

  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/slicerExtensionWizard-install.sh
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    COMPONENT Runtime
    RENAME slicerExtensionWizard
    )

  # Install Python libexec scripts
  set(Slicer_LIBEXEC_PYTHON_SCRIPTS
    ExtensionWizard.py
    )

  ctkMacroCompilePythonScript(
    TARGET_NAME ExecScripts
    SCRIPTS "${Slicer_LIBEXEC_PYTHON_SCRIPTS}"
    DESTINATION_DIR ${Slicer_BINARY_DIR}/libexec
    INSTALL_DIR ${Slicer_INSTALL_LIBEXEC_DIR}
    NO_INSTALL_SUBDIR
    )
endif()
