
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

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  if(NOT DEFINED MY_NAME)
    message(SEND_ERROR "NAME is mandatory")
  endif()

  if(DEFINED MY_LOGO_HEADER AND NOT EXISTS ${MY_LOGO_HEADER})
    message(SEND_ERROR "Specified LOGO_HEADER [${header}] doesn't exist")
    set(MY_LOGO_HEADER)
  endif()
  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MY_NAME}.xml)
    message(SEND_ERROR "Xml file [${MY_NAME}.xml] doesn't exist !")
  endif()

  set(CLP ${MY_NAME})

  # GenerateCLP
  find_package(GenerateCLP REQUIRED)
  include(${GenerateCLP_USE_FILE})

  set(${CLP}_SOURCE ${CLP}.cxx ${MY_ADDITIONAL_SRCS})
  generateclp(${CLP}_SOURCE ${CLP}.xml ${MY_LOGO_HEADER})

  if(DEFINED MY_LINK_DIRECTORIES)
    link_directories(${MY_LINK_DIRECTORIES})
  endif()

  if(DEFINED MY_INCLUDE_DIRECTORIES)
    include_directories(${MY_INCLUDE_DIRECTORIES})
  endif()

  if(NOT MY_EXECUTABLE_ONLY)

    add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
    set_target_properties(${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
    slicer3_set_plugins_output_path(${CLP}Lib)
    if(DEFINED MY_TARGET_LIBRARIES)
      target_link_libraries(${CLP}Lib ${MY_TARGET_LIBRARIES})
    endif()

    # Set labels associated with the target.
    set_target_properties(${CLP}Lib PROPERTIES LABELS ${CLP})

    add_executable(${CLP} ${Slicer_SOURCE_DIR}/Applications/CLI/Templates/CommandLineSharedLibraryWrapper.cxx)
    slicer3_set_plugins_output_path(${CLP})
    target_link_libraries(${CLP} ${CLP}Lib)

    # Set labels associated with the target.
    set_target_properties(${CLP} PROPERTIES LABELS ${CLP})

    # Install each target in the production area (where it would appear in an installation)
    # and install each target in the developer area (for running from a build)
    set(TARGETS ${CLP} ${CLP}Lib)
    slicer3_install_plugins(${TARGETS})

  else()

    add_executable(${CLP} ${${CLP}_SOURCE})
    slicer3_set_plugins_output_path(${CLP})
    if(DEFINED MY_TARGET_LIBRARIES)
      target_link_libraries(${CLP} ${MY_TARGET_LIBRARIES})
    endif()

    # Set labels associated with the target.
    set_target_properties(${CLP} PROPERTIES LABELS ${CLP})

    # Install each target in the production area (where it would appear in an installation)
    # and install each target in the developer area (for running from a build)
    set(TARGETS ${CLP})
    slicer3_install_plugins(${TARGETS})
  endif()

endmacro()
