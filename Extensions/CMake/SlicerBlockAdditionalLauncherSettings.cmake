

#
# In case the extension is build using a multi-configuration system, the build
# type will be know at build. For that reason, a script allowing to configure
# the "additonal launcher settings" at build time will be generated.
#

if(NOT TARGET ConfigureAdditionalLauncherSettings)

  set(Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE ${CMAKE_CURRENT_BINARY_DIR}/AdditionalLauncherSettings.ini)
  set(Slicer_ADDITIONAL_LAUNCHER_SETTINGS "--launcher-additional-settings" ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE})

  # Add '--launcher-additional-settings' to launch command
  list(FIND Slicer_LAUNCH_COMMAND "--launch" launch_index)
  list(INSERT Slicer_LAUNCH_COMMAND ${launch_index} ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS})

  list(FIND SEM_LAUNCH_COMMAND "--launch" launch_index)
  list(INSERT SEM_LAUNCH_COMMAND ${launch_index} ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS})

  # Configure script

  # Third party libraries may live at the superbuild top level, add them to
  # the paths lists and adjust the number of paths size, otherwise just use
  # the local paths
  if (DEFINED EXTENSION_SUPERBUILD_BINARY_DIR)
    set(THIRD_PARTY_LIBRARYPATHS "3\\\\path=${EXTENSION_SUPERBUILD_BINARY_DIR}/${Slicer_THIRDPARTY_LIB_DIR}/\${CMAKE_CFG_INTDIR}
size=3")
    set(THIRD_PARTY_BINPATHS "3\\\\path=${EXTENSION_SUPERBUILD_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}/\${CMAKE_CFG_INTDIR}
size=3")
    set(THIRD_PARTY_PYTHONPATHS "4\\\\path=${EXTENSION_SUPERBUILD_BINARY_DIR}/${Slicer_THIRDPARTY_LIB_DIR}/\${CMAKE_CFG_INTDIR}
size=4")
  else()
    set(THIRD_PARTY_LIBRARYPATHS "size=2")
    set(THIRD_PARTY_BINPATHS "size=2")
    set(THIRD_PARTY_PYTHONPATHS "size=3")
  endif()

  set(_additonal_settings_configure_script ${CMAKE_CURRENT_BINARY_DIR}/AdditionalLauncherSettings-configure.cmake)
  file(WRITE ${_additonal_settings_configure_script}
  "
  file(WRITE ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE}
\"[LibraryPaths]
1\\\\path=${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR}/\${CMAKE_CFG_INTDIR}
2\\\\path=${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/\${CMAKE_CFG_INTDIR}
${THIRD_PARTY_LIBRARYPATHS}

[Paths]
1\\\\path=${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}/\${CMAKE_CFG_INTDIR}
2\\\\path=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}/\${CMAKE_CFG_INTDIR}
${THIRD_PARTY_BINPATHS}

[PYTHONPATH]
1\\\\path=${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
2\\\\path=${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/\${CMAKE_CFG_INTDIR}
3\\\\path=${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}
${THIRD_PARTY_PYTHONPATHS}
\")
")

  add_custom_command(
    DEPENDS
      ${CMAKE_CURRENT_LIST_FILE}
    OUTPUT
      ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE}
    COMMAND ${CMAKE_COMMAND}
      -DCMAKE_CFG_INTDIR:STRING=${CMAKE_CFG_INTDIR}
      -P ${_additonal_settings_configure_script}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Configuring: AdditionalLauncherSettings.ini"
    )

  add_custom_target(ConfigureAdditionalLauncherSettings ALL
    DEPENDS
      ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE}
    )
    
endif()
