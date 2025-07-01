
set(proj PCRE2)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED PCRE2_DIR AND NOT EXISTS ${PCRE2_DIR})
  message(FATAL_ERROR "PCRE2_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  #
  #  PCRE (Perl Compatible Regular Expressions)
  #

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/PCRE2)
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/PCRE2-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/PCRE2-install)

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

  set(_version "10.44")

  ExternalProject_add(PCRE2
    ${${proj}_EP_ARGS}
    URL https://github.com/Slicer/SlicerBinaryDependencies/releases/download/PCRE2/pcre2-${_version}.tar.gz
    URL_HASH SHA512=c43bbe2235993cd703e887bc48cf76d6a6d2f8377accfee3620eff5a4681a36e1f9c147d9e99c6674d5648a54af1d5d174018d9a102abbea7e8d38337bbbd17e
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

  set(PCRE2_DIR ${EP_INSTALL_DIR})
endif()
