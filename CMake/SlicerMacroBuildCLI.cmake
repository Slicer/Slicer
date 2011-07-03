
#
# Depends on:
#  Slicer/CMake/SlicerMacroParseArguments.cmake
#

macro(slicerMacroBuildCLI)
  SLICER_PARSE_ARGUMENTS(MY
    "NAME;ADDITIONAL_SRCS;LOGO_HEADER;TARGET_LIBRARIES;LINK_DIRECTORIES;INCLUDE_DIRECTORIES"
    "EXECUTABLE_ONLY"
    ${ARGN}
    )
  message(STATUS "Configuring CLI module: ${MY_NAME}")
  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  if(NOT DEFINED MY_NAME)
    message(FATAL_ERROR "error: NAME is mandatory")
  endif()

  if(DEFINED MY_LOGO_HEADER AND NOT EXISTS ${MY_LOGO_HEADER})
    message(WARNING "warning: Specified LOGO_HEADER [${header}] doesn't exist")
    set(MY_LOGO_HEADER)
  endif()

  foreach(v Slicer_CLI_SHARED_LIBRARY_WRAPPER_CXX)
    if(NOT EXISTS "${${v}}")
      message(FATAL_ERROR "error: Variable ${v} point to an non-existing file or directory !")
    endif()
  endforeach()

  set(cli_xml_file ${CMAKE_CURRENT_SOURCE_DIR}/${MY_NAME}.xml)
  if(NOT EXISTS ${cli_xml_file})
    set(cli_xml_file ${CMAKE_CURRENT_BINARY_DIR}/${MY_NAME}.xml)
    if(NOT EXISTS ${cli_xml_file})
      message(FATAL_ERROR "Xml file [${MY_NAME}.xml] doesn't exist !")
    endif()
  endif()

  set(CLP ${MY_NAME})

  # GenerateCLP
  find_package(GenerateCLP REQUIRED)
  include(${GenerateCLP_USE_FILE})

  set(${CLP}_SOURCE ${CLP}.cxx ${MY_ADDITIONAL_SRCS})
  generateclp(${CLP}_SOURCE ${cli_xml_file} ${MY_LOGO_HEADER})

  if(DEFINED MY_LINK_DIRECTORIES)
    link_directories(${MY_LINK_DIRECTORIES})
  endif()

  if(DEFINED MY_INCLUDE_DIRECTORIES)
    include_directories(${MY_INCLUDE_DIRECTORIES})
  endif()

  set(cli_targets)

  if(NOT MY_EXECUTABLE_ONLY)

    add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
    set_target_properties(${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
    if(DEFINED MY_TARGET_LIBRARIES)
      target_link_libraries(${CLP}Lib ${MY_TARGET_LIBRARIES})
    endif()

    add_executable(${CLP} ${Slicer_CLI_SHARED_LIBRARY_WRAPPER_CXX})
    target_link_libraries(${CLP} ${CLP}Lib)

    set(cli_targets ${CLP} ${CLP}Lib)

  else()

    add_executable(${CLP} ${${CLP}_SOURCE})
    if(DEFINED MY_TARGET_LIBRARIES)
      target_link_libraries(${CLP} ${MY_TARGET_LIBRARIES})
    endif()

    set(cli_targets ${CLP})

  endif()

  # Set labels associated with the target.
  set_target_properties(${cli_targets} PROPERTIES LABELS ${CLP})

  set_target_properties(${cli_targets} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_PLUGINS_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_PLUGINS_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_PLUGINS_LIB_DIR}"
    )

  # Install each target in the production area (where it would appear in an installation)
  # and install each target in the developer area (for running from a build)
  install(TARGETS ${cli_targets}
    RUNTIME DESTINATION ${Slicer_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
    )


endmacro()
