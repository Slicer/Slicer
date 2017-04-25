

#
# In case the extension is build using a multi-configuration system, the build
# type will be know at build. For that reason, a script allowing to configure
# the "additional launcher settings" at build time will be generated.
#

if(NOT TARGET ConfigureAdditionalLauncherSettings)

  set(Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE ${CMAKE_CURRENT_BINARY_DIR}/AdditionalLauncherSettings.ini)
  set(Slicer_ADDITIONAL_LAUNCHER_SETTINGS "--launcher-additional-settings" ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE})

  # Add '--launcher-additional-settings' to launch command
  list(FIND Slicer_LAUNCH_COMMAND "--launch" launch_index)
  list(INSERT Slicer_LAUNCH_COMMAND ${launch_index} ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS})

  list(FIND SEM_LAUNCH_COMMAND "--launch" launch_index)
  list(INSERT SEM_LAUNCH_COMMAND ${launch_index} ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS})

  include(${CTKAPPLAUNCHER_DIR}/CMake/ctkAppLauncher.cmake)

  #-----------------------------------------------------------------------------
  #-----------------------------------------------------------------------------
  # Settings specific to the build tree.
  #-----------------------------------------------------------------------------
  #-----------------------------------------------------------------------------

  #-----------------------------------------------------------------------------
  # LIBRARY_PATHS
  #-----------------------------------------------------------------------------
  set(EXTENSION_LIBRARY_PATHS_BUILD
    ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR}/\${CMAKE_CFG_INTDIR}
    ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/\${CMAKE_CFG_INTDIR}
    )

  # Third party libraries may live at the superbuild top level.
  if(DEFINED EXTENSION_SUPERBUILD_BINARY_DIR)
    list(APPEND EXTENSION_LIBRARY_PATHS_BUILD
      ${EXTENSION_SUPERBUILD_BINARY_DIR}/${Slicer_THIRDPARTY_LIB_DIR}/\${CMAKE_CFG_INTDIR}
      )
  endif()

  #-----------------------------------------------------------------------------
  # PATHS
  #-----------------------------------------------------------------------------
  set(EXTENSION_PATHS_BUILD
    ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}/\${CMAKE_CFG_INTDIR}
    ${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}/\${CMAKE_CFG_INTDIR}
    )

  # Third party libraries may live at the superbuild top level.
  if(DEFINED EXTENSION_SUPERBUILD_BINARY_DIR)
    list(APPEND EXTENSION_PATHS_BUILD
      ${EXTENSION_SUPERBUILD_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}/\${CMAKE_CFG_INTDIR}
      )
  endif()

  #-----------------------------------------------------------------------------
  # ENVVARS
  #-----------------------------------------------------------------------------
  set(EXTENSION_ENVVARS_BUILD
    "PYTHONPATH"
    )

  set(EXTENSION_PYTHONPATH_BUILD
    ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
    ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/\${CMAKE_CFG_INTDIR}
    ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}
    )
  # Third party libraries may live at the superbuild top level.
  if(DEFINED EXTENSION_SUPERBUILD_BINARY_DIR)
    list(APPEND EXTENSION_PYTHONPATH_BUILD
      ${EXTENSION_SUPERBUILD_BINARY_DIR}/${Slicer_THIRDPARTY_LIB_DIR}/\${CMAKE_CFG_INTDIR}
      )
  endif()

  #-----------------------------------------------------------------------------
  #-----------------------------------------------------------------------------
  # Generate script allowing to configure AdditionalLauncherSettings.ini at build time
  #-----------------------------------------------------------------------------
  #-----------------------------------------------------------------------------

  # library paths
  set(EXTENSION_LAUNCHER_SETTINGS_LIBRARY_PATHS)
  ctkAppLauncherListToQtSettingsArray(
    "${EXTENSION_LIBRARY_PATHS_BUILD}" "path" EXTENSION_LAUNCHER_SETTINGS_LIBRARY_PATHS)

  # paths
  set(EXTENSION_LAUNCHER_SETTINGS_PATHS)
  ctkAppLauncherListToQtSettingsArray(
    "${EXTENSION_PATHS_BUILD}" "path" EXTENSION_LAUNCHER_SETTINGS_PATHS)

  # env. variables
  set(EXTENSION_LAUNCHER_SETTINGS_ADDITIONAL_PATHS)
  foreach(envvar ${EXTENSION_ENVVARS_BUILD})
    set(cmake_varname EXTENSION_${envvar}_BUILD)
    ctkAppLauncherListToQtSettingsArray("${${cmake_varname}}" "path" _extension_paths_${envvar})
    set(EXTENSION_LAUNCHER_SETTINGS_ADDITIONAL_PATHS "${EXTENSION_LAUNCHER_SETTINGS_ADDITIONAL_PATHS}

[${envvar}]
${_extension_paths_${envvar}}")
  endforeach()

  # Write script
  set(_additional_settings_configure_script ${CMAKE_CURRENT_BINARY_DIR}/AdditionalLauncherSettings-configure.cmake)
  file(WRITE ${_additional_settings_configure_script}
  "
  file(WRITE ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE}
\"[LibraryPaths]
${EXTENSION_LAUNCHER_SETTINGS_LIBRARY_PATHS}

[Paths]
${EXTENSION_LAUNCHER_SETTINGS_PATHS}

${EXTENSION_LAUNCHER_SETTINGS_ADDITIONAL_PATHS}
\")
")

  add_custom_command(
    DEPENDS
      ${CMAKE_CURRENT_LIST_FILE}
    OUTPUT
      ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE}
    COMMAND ${CMAKE_COMMAND}
      -DCMAKE_CFG_INTDIR:STRING=${CMAKE_CFG_INTDIR}
      -P ${_additional_settings_configure_script}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Configuring: AdditionalLauncherSettings.ini"
    )

  add_custom_target(ConfigureAdditionalLauncherSettings ALL
    DEPENDS
      ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE}
    )
    
endif()
