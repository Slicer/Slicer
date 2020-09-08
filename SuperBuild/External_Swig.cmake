
set(proj Swig)

if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED Swig_DIR AND NOT EXISTS ${Swig_DIR})
  message(FATAL_ERROR "Swig_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT SWIG_DIR AND NOT Slicer_USE_SYSTEM_${proj})

  set(SWIG_TARGET_VERSION 4.0.2)
  set(SWIG_DOWNLOAD_SOURCE_HASH "05e7da70ce6d9a733b96c0bcfa3c1b82765bd859f48c74759bbf4bb1467acb1809caa310cba5e2b3280cd704fca249eaa0624821dffae1d2a75097c7f55d14ed")
  set(SWIG_DOWNLOAD_WIN_HASH "b8f105f9b9db6acc1f6e3741990915b533cd1bc206eb9645fd6836457fd30789b7229d2e3219d8e35f2390605ade0fbca493ae162ec3b4bc4e428b57155db03d")

  if(WIN32)
    set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION})

    # swig.exe available as pre-built binary on Windows:
    ExternalProject_Add(Swig
      URL https://github.com/Slicer/SlicerBinaryDependencies/releases/download/swig/swigwin-${SWIG_TARGET_VERSION}.zip
      URL_HASH SHA512=${SWIG_DOWNLOAD_WIN_HASH}
      SOURCE_DIR "${EP_BINARY_DIR}"
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      UPDATE_COMMAND ""
      )

    set(SWIG_DIR "${EP_BINARY_DIR}") # path specified as source in ep
    set(SWIG_EXECUTABLE "${EP_BINARY_DIR}/swig.exe")
    set(Swig_DEPEND Swig)

  else()
    # not windows

    # Set dependency list
    set(${proj}_DEPENDENCIES PCRE python)

    # Include dependent projects if any
    ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

    #
    # SWIG
    #

    # swig uses bison find it by cmake and pass it down
    # Note: Based on Swig external project found in SimpleITK and the official swig
    #       documentation, Bison is not required when building Swig from a pre-generated
    #       source distribution.
    find_package(BISON QUIET)
    set(BISON_FLAGS "" CACHE STRING "Flags used by bison")
    mark_as_advanced(BISON_FLAGS)

    set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/Swig)
    set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/Swig-build)
    set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/Swig-install)

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
    --prefix=${EP_INSTALL_DIR}
    --with-pcre-prefix=${PCRE_DIR}
    --without-octave
    --without-java
    --with-python=${PYTHON_EXECUTABLE})
")

    ExternalProject_add(Swig
      URL https://github.com/Slicer/SlicerBinaryDependencies/releases/download/swig/swig-${SWIG_TARGET_VERSION}.tar.gz
      URL_HASH SHA512=${SWIG_DOWNLOAD_SOURCE_HASH}
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
      SOURCE_DIR ${EP_SOURCE_DIR}
      BINARY_DIR ${EP_BINARY_DIR}
      CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${_configure_script}
      INSTALL_COMMAND make install -j1  # Avoid race condition disabling parallel build
      DEPENDS ${${proj}_DEPENDENCIES}
      )

    set(SWIG_DIR "${EP_INSTALL_DIR}/share/swig/${SWIG_TARGET_VERSION}")
    set(SWIG_EXECUTABLE ${EP_INSTALL_DIR}/bin/swig)
    set(Swig_DEPEND Swig)
  endif()

  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${SWIG_TARGET_VERSION}
    )

  mark_as_superbuild(
    VARS SWIG_EXECUTABLE:FILEPATH
    LABELS "FIND_PACKAGE"
    )
endif()
