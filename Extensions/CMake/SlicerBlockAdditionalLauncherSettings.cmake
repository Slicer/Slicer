

#
# In case the extension is build using a multi-configuration system, the build
# type will be know at build. For that reason, a script allowing to configure
# the "additional launcher settings" at build time will be generated.
#

# Is the extension/module using SuperBuild ?
set(_use_superbuild 0)
if(DEFINED EXTENSION_BUILD_SUBDIRECTORY AND NOT "${EXTENSION_BUILD_SUBDIRECTORY}" STREQUAL ".")
  set(_use_superbuild 1)
endif()

# Is the top-level project being configured ?
set(_configuring_top_level_project 1)
if(_use_superbuild AND DEFINED ${EXTENSION_NAME}_SUPERBUILD AND NOT "${${EXTENSION_NAME}_SUPERBUILD}")
  set(_configuring_top_level_project 0)
endif()

# Configure additional launcher settings ?
if(NOT _use_superbuild)
  set(_msg_status "yes")
  set(_configure_additional_launcher_settings 1)
else()
  if(_configuring_top_level_project)
    set(_msg_status "no (because configuring top-level project)")
    set(_configure_additional_launcher_settings 0)
  else()
    set(_msg_status "yes (because configuring ${EXTENSION_BUILD_SUBDIRECTORY})")
    set(_configure_additional_launcher_settings 1)
  endif()
endif()

set(_msg "Adding ConfigureAdditionalLauncherSettings target")
message(STATUS "${_msg}")
message(STATUS "${_msg} - ${_msg_status}")

if(NOT TARGET ConfigureAdditionalLauncherSettings AND _configure_additional_launcher_settings)

  # Load additional paths variables from extensions that we depend on.
  if (EXTENSION_DEPENDS)
    # If needed, convert to a list
    list(LENGTH EXTENSION_DEPENDS _count)
    if(_count EQUAL 1)
      string(REPLACE " " ";" EXTENSION_DEPENDS ${EXTENSION_DEPENDS})
    endif()
    foreach(dep ${EXTENSION_DEPENDS})
      # When no extension dependencies are specified, "NA" value may be listed, it should be ignored.
      if (NOT "${dep}" STREQUAL "NA")
        find_package(${dep} QUIET)
        if (NOT ${dep}_FOUND)
          message(WARNING "Dependent extension ${dep} cannot be found by CMake find_package(), therefore paths variables cannot be imported from this extension. The problem can be resolved by generating ${dep}Config.cmake by adding include(\${Slicer_EXTENSION_GENERATE_CONFIG}) to the top-level CMakeLists.txt of the dependent extension.")
        endif()
      endif()
    endforeach()
  endif()

  set(Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE ${CMAKE_CURRENT_BINARY_DIR}/AdditionalLauncherSettings.ini)
  set(Slicer_ADDITIONAL_LAUNCHER_SETTINGS "--launcher-additional-settings" ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE})

  # Add '--launcher-additional-settings' to launch command
  list(FIND Slicer_LAUNCH_COMMAND "--launch" launch_index)
  list(INSERT Slicer_LAUNCH_COMMAND ${launch_index} ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS})

  list(FIND SEM_LAUNCH_COMMAND "--launch" launch_index)
  list(INSERT SEM_LAUNCH_COMMAND ${launch_index} ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS})

  include(${CTKAppLauncher_DIR}/CMake/ctkAppLauncher.cmake)

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

  # External projects - library paths
  foreach(varname IN LISTS ${SUPERBUILD_TOPLEVEL_PROJECT}_EP_LABEL_LIBRARY_PATHS_LAUNCHER_BUILD)
    list(APPEND EXTENSION_LIBRARY_PATHS_BUILD ${${varname}})
  endforeach()

  # Extension dependencies - library paths
  foreach(dep ${EXTENSION_DEPENDS})
    string(REPLACE "\$(Configuration)" "\${CMAKE_CFG_INTDIR}" path "${${dep}_LIBRARY_PATHS_LAUNCHER_BUILD}")
    list(APPEND EXTENSION_LIBRARY_PATHS_BUILD ${path})
  endforeach()

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

  # External projects - paths
  foreach(varname IN LISTS ${SUPERBUILD_TOPLEVEL_PROJECT}_EP_LABEL_PATHS_LAUNCHER_BUILD)
    list(APPEND EXTENSION_PATHS_BUILD ${${varname}})
  endforeach()

  # Extension dependencies - paths
  foreach(dep ${EXTENSION_DEPENDS})
    string(REPLACE "\$(Configuration)" "\${CMAKE_CFG_INTDIR}" path "${${dep}_PATHS_LAUNCHER_BUILD}")
    list(APPEND EXTENSION_PATHS_BUILD ${path})
  endforeach()

  #-----------------------------------------------------------------------------
  # ENVVARS
  #-----------------------------------------------------------------------------

  set(EXTENSION_LAUNCHER_SETTINGS_ENVVARS)

  # External projects - environment variables
  foreach(varname IN LISTS ${SUPERBUILD_TOPLEVEL_PROJECT}_EP_LABEL_ENVVARS_LAUNCHER_BUILD)
    list(APPEND EXTENSION_LAUNCHER_SETTINGS_ENVVARS ${${varname}})
  endforeach()

  # Extension dependencies - environment variables
  foreach(dep ${EXTENSION_DEPENDS})
    string(REPLACE "\$(Configuration)" "\${CMAKE_CFG_INTDIR}" path "${${dep}_ENVVARS_LAUNCHER_BUILD}")
    list(APPEND EXTENSION_LAUNCHER_SETTINGS_ENVVARS ${path})
  endforeach()

  #-----------------------------------------------------------------------------
  # PATH ENVVARS
  #-----------------------------------------------------------------------------

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

  # External projects - pythonpath
  foreach(varname IN LISTS ${SUPERBUILD_TOPLEVEL_PROJECT}_EP_LABEL_PYTHONPATH_LAUNCHER_BUILD)
    list(APPEND EXTENSION_PYTHONPATH_BUILD ${${varname}})
  endforeach()

  # Extension dependencies - pythonpath
  foreach(dep ${EXTENSION_DEPENDS})
    string(REPLACE "\$(Configuration)" "\${CMAKE_CFG_INTDIR}" path "${${dep}_PYTHONPATH_LAUNCHER_BUILD}")
    list(APPEND EXTENSION_PYTHONPATH_BUILD ${path})
  endforeach()

  set(EXTENSION_PATH_ENVVARS_BUILD
    "PYTHONPATH"
    )

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
  set(EXTENSION_LAUNCHER_SETTINGS_ENVVARS)
  foreach(envvar ${EXTENSION_ENVVARS_BUILD})
    set(EXTENSION_LAUNCHER_SETTINGS_ENVVARS "${EXTENSION_LAUNCHER_SETTINGS_ENVVARS}${envvar}\n")
  endforeach()

  # paths env. variables
  string(REPLACE ";" "," EXTENSION_LAUNCHER_SETTINGS_PATH_ENVVARS "${EXTENSION_PATH_ENVVARS_BUILD}")

  set(EXTENSION_LAUNCHER_SETTINGS_ADDITIONAL_PATH_ENVVARS)
  foreach(envvar ${EXTENSION_PATH_ENVVARS_BUILD})
    set(cmake_varname EXTENSION_${envvar}_BUILD)
    ctkAppLauncherListToQtSettingsArray("${${cmake_varname}}" "path" _extension_paths_${envvar})
    set(EXTENSION_LAUNCHER_SETTINGS_ADDITIONAL_PATH_ENVVARS "${EXTENSION_LAUNCHER_SETTINGS_ADDITIONAL_PATH_ENVVARS}

[${envvar}]
${_extension_paths_${envvar}}")
  endforeach()

  # Write script
  set(_additional_settings_configure_script ${CMAKE_CURRENT_BINARY_DIR}/AdditionalLauncherSettings-configure.cmake)
  file(WRITE ${_additional_settings_configure_script}
  "file(WRITE ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS_FILE}
\"[General]
additionalPathVariables=${EXTENSION_LAUNCHER_SETTINGS_PATH_ENVVARS}

[LibraryPaths]
${EXTENSION_LAUNCHER_SETTINGS_LIBRARY_PATHS}

[Paths]
${EXTENSION_LAUNCHER_SETTINGS_PATHS}

[EnvironmentVariables]
${EXTENSION_LAUNCHER_SETTINGS_ENVVARS}

${EXTENSION_LAUNCHER_SETTINGS_ADDITIONAL_PATH_ENVVARS}
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
