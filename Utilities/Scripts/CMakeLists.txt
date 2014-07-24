if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
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

  if(WIN32)
    set(EXPORT_GIT_PYTHON_GIT_EXECUTABLE_CONFIG
"\# Export path to git executable provided by Git Bash
export GIT_PYTHON_GIT_EXECUTABLE=/bin/git")
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
