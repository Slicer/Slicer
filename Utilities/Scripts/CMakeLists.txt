if(UNIX AND NOT APPLE)
  # Install generic script to restore the original system environment when
  # launching system tools (e.g., `git`) from within a Slicer launcher context.
  configure_file(
    exec-outside-slicer-env.sh
    ${Slicer_BINARY_DIR}/bin/exec-outside-slicer-env.sh
    COPYONLY
    USE_SOURCE_PERMISSIONS
    )
  install(PROGRAMS ${Slicer_BINARY_DIR}/bin/exec-outside-slicer-env.sh
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    COMPONENT Runtime
    )
  if(BUILD_TESTING)
    get_filename_component(cmake_directory ${CMAKE_COMMAND} DIRECTORY)
    set(test_env "PATH=${cmake_directory}:$ENV{PATH}")

    # Precondition: Ensure APPLAUNCHER_* variables are set using the launcher
    set(test_name "sh-exec-outside-slicer-env-precondition-test")
    add_test(
      NAME ${test_name}
      COMMAND ${Slicer_LAUNCH_COMMAND}
        cmake -E environment
      )
    set_tests_properties(${test_name} PROPERTIES
      ENVIRONMENT "${test_env}"
      FAIL_REGULAR_EXPRESSION "Skipping environment restoration"
      PASS_REGULAR_EXPRESSION "APPLAUNCHER_"
      )

    # Check that exec-outside-slicer-env unsets APPLAUNCHER_* variables
    set(test_name "sh-exec-outside-slicer-env-test")
    add_test(
      NAME ${test_name}
      COMMAND ${Slicer_LAUNCH_COMMAND}
        ${Slicer_BINARY_DIR}/bin/exec-outside-slicer-env.sh cmake -E environment
      )
    set_tests_properties(${test_name} PROPERTIES
      ENVIRONMENT "${test_env}"
      FAIL_REGULAR_EXPRESSION "APPLAUNCHER_;Skipping environment restoration"
      )
  endif()

  if(Slicer_BUILD_KRB5_GSSAPI_STUB)
    # Install wrappers for system executables to ensure they are called
    # with a clean environment. This avoids symbol resolution errors (e.g.,
    # in `libssh`) caused by the GSSAPI stub.
    #
    # See: https://github.com/Slicer/Slicer/issues/8612
    foreach(tool_name IN ITEMS
        git
      )
      configure_file(
        system-executable-wrapper.sh.in
        ${Slicer_BINARY_DIR}/bin/${tool_name}
        COPYONLY
        USE_SOURCE_PERMISSIONS
        )

      install(PROGRAMS ${Slicer_BINARY_DIR}/bin/${tool_name}
        DESTINATION ${Slicer_INSTALL_BIN_DIR}
        COMPONENT Runtime
        )
    endforeach()

    if(BUILD_TESTING)
      get_filename_component(git_directory ${GIT_EXECUTABLE} DIRECTORY)
      set(test_env
        "PATH=${git_directory}:$ENV{PATH}"
        "SLICER_DEBUG_KRB5_GSSAPI_STUB=1"
        )

      # Precondition: Verify krb5-gssapi-stub is injected in Git without wrapper
      set(test_name "sh-git-executable-wrapper-ls-remote-precondition-test")
      add_test(
        NAME ${test_name}
        COMMAND ${Slicer_LAUNCH_COMMAND}
          ${GIT_EXECUTABLE} ls-remote -t https://github.com/Slicer/Slicer
        )
      set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "${test_env}"
        PASS_REGULAR_EXPRESSION "krb5-gssapi-stub;undefined symbol"
        )

      # Verify wrapper restores environment and invokes system Git correctly
      set(test_name "sh-git-executable-wrapper-le-remote-test")
      add_test(
        NAME ${test_name}
        COMMAND ${Slicer_LAUNCH_COMMAND}
          ${Slicer_BINARY_DIR}/bin/git ls-remote -t https://github.com/Slicer/Slicer
        )
      set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "${test_env}"
        FAIL_REGULAR_EXPRESSION "krb5-gssapi-stub;Skipping environment restoration"
        PASS_REGULAR_EXPRESSION "refs/tags"
        )
    endif()
  endif()
endif()

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT AND Slicer_USE_PYTHONQT)
  # Determine name of Python interpreter
  #
  # NOTE: Do NOT use ${Python3_EXECUTABLE}; when using our own Python, it is set
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
