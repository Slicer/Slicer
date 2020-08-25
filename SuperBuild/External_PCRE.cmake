
set(proj PCRE)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED PCRE_DIR AND NOT EXISTS ${PCRE_DIR})
  message(FATAL_ERROR "PCRE_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  #
  #  PCRE (Perl Compatible Regular Expressions)
  #

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/PCRE)
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/PCRE-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/PCRE-install)

  include(ExternalProjectForNonCMakeProject)

  # environment
  set(_env_script ${CMAKE_BINARY_DIR}/${proj}_Env.cmake)
  ExternalProject_Write_SetBuildEnv_Commands(${_env_script})
  file(APPEND ${_env_script}
"#------------------------------------------------------------------------------
# Added by '${CMAKE_CURRENT_LIST_FILE}'

set(ENV{YACC} \"${BISON_EXECUTABLE}\")
set(ENV{YFLAGS} \"${BISON_FLAGS}\")
")

  # configure step
  set(_configure_script ${CMAKE_BINARY_DIR}/${proj}_configure_step.cmake)
  file(WRITE ${_configure_script}
"include(\"${_env_script}\")
set(${proj}_WORKING_DIR \"${EP_BINARY_DIR}\")
ExternalProject_Execute(${proj} \"configure\" sh ${EP_SOURCE_DIR}/configure
    --prefix=${EP_INSTALL_DIR} --disable-shared)
")

  set(_version "8.44")

  ExternalProject_add(PCRE
    ${${proj}_EP_ARGS}
    URL https://github.com/Slicer/SlicerBinaryDependencies/releases/download/PCRE/pcre-${_version}.tar.gz
    URL_HASH SHA512=abac4c4f9df9e61d7d7761a9c50843882611752e1df0842a54318f358c28f5953025eba2d78997d21ee690756b56cc9f1c04a5ed591dd60654cc78ba16d9ecfb
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    UPDATE_COMMAND "" # Disable update
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_script}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${_version}
    )

  set(PCRE_DIR ${EP_INSTALL_DIR})
endif()
