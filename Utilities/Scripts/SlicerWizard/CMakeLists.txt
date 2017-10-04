if("${Slicer_RELEASE_TYPE}" STREQUAL "Stable")
  set(Slicer_VERSION_TWEAK_OR_DEV "0")
else()
  set(Slicer_VERSION_TWEAK_OR_DEV "\"dev0\"")
endif()

# Yes, this file is configured in the SOURCE directory. And yes it is checked
# in. The contents of the generated file should only be changed when the Slicer
# version number is changed, and the updated __version__.py should be committed
# at the same time. This is mainly to ensure that __version__.py is kept in
# sync.
configure_file(
  "__version__.py.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/__version__.py"
  @ONLY
  NEWLINE_STYLE UNIX)

set(SlicerWizard_PYTHON_SCRIPTS
  __init__.py
  __version__.py
  CMakeParser.py
  ExtensionDescription.py
  ExtensionProject.py
  ExtensionWizard.py
  GithubHelper.py
  Subversion.py
  TemplateManager.py
  Utilities.py
  WizardHelpFormatter.py
  )

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT AND Slicer_USE_PYTHONQT)
  ctkMacroCompilePythonScript(
    TARGET_NAME SlicerWizard
    SCRIPTS "${SlicerWizard_PYTHON_SCRIPTS}"
    DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python/SlicerWizard
    INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}/Python
    )
endif()
