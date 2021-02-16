macro(SlicerMacroBuildScriptedCLI)
  set(options
    NO_INSTALL
    VERBOSE
    )
  set(oneValueArgs
    NAME
    FOLDER
    )
  set(multiValueArgs
    )
  cmake_parse_arguments(LOCAL_SEM
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  message(STATUS "Configuring CLI Scripted module: ${LOCAL_SEM_NAME}")

  # --------------------------------------------------------------------------
  # Print information helpful for debugging checks
  # --------------------------------------------------------------------------
  if(LOCAL_SEM_VERBOSE)
    foreach(curr_opt IN LISTS options oneValueArgs multiValueArgs)
      message(STATUS "${curr_opt} = ${LOCAL_SEM_${curr_opt}}")
    endforeach()
  endif()

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  if(LOCAL_SEM_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to SEMMacroBuildScriptedCLI(): \"${LOCAL_SEM_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT DEFINED LOCAL_SEM_NAME)
    message(FATAL_ERROR "NAME is mandatory")
  endif()

  set(cli_script_file "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SEM_NAME}.py")
  set(cli_xml_file "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SEM_NAME}.xml")

  set(binary_dir "${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}")

  if(CMAKE_CONFIGURATION_TYPES)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.20")
      set(binary_dir "${binary_dir}/$<CONFIG>")
    else()
      set(binary_dir "${binary_dir}/${CMAKE_CFG_INTDIR}")
    endif()
  endif()

  add_custom_command(
    OUTPUT "${binary_dir}/${LOCAL_SEM_NAME}.py" "${binary_dir}/${LOCAL_SEM_NAME}.xml"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${binary_dir}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${cli_script_file}" "${binary_dir}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${cli_xml_file}" "${binary_dir}"
    DEPENDS "${cli_script_file}" "${cli_xml_file}"
    COMMENT "Copying ${LOCAL_SEM_NAME} files to ${binary_dir}"
    )

  add_custom_target(${LOCAL_SEM_NAME} ALL
    SOURCES "${cli_script_file}" "${cli_xml_file}"
    DEPENDS "${binary_dir}/${LOCAL_SEM_NAME}.py" "${binary_dir}/${LOCAL_SEM_NAME}.xml"
    )

  set(cli_targets ${LOCAL_SEM_NAME})

  if(NOT LOCAL_SEM_NO_INSTALL)
    install(
      FILES
        ${cli_script_file}
        ${cli_xml_file}
      DESTINATION "${Slicer_INSTALL_CLIMODULES_BIN_DIR}"
      COMPONENT Runtime
      )
  endif()

  # Folder
  if(NOT DEFINED LOCAL_SEM_FOLDER)
    set(LOCAL_SEM_FOLDER "${SlicerExecutionModel_DEFAULT_CLI_TARGETS_FOLDER_PREFIX}${LOCAL_SEM_NAME}")
  endif()
  foreach(target ${cli_targets})
    set_target_properties(${target} PROPERTIES FOLDER ${LOCAL_SEM_FOLDER})
  endforeach()
  set(${LOCAL_SEM_NAME}_TARGETS_FOLDER ${LOCAL_SEM_FOLDER})

endmacro()
