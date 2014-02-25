
set(proj Swig)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED Swig_DIR AND NOT EXISTS ${Swig_DIR})
  message(FATAL_ERROR "Swig_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT SWIG_DIR AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  set(SWIG_TARGET_VERSION 2.0.12)
  set(SWIG_DOWNLOAD_SOURCE_HASH "c3fb0b2d710cc82ed0154b91e43085a4")
  set(SWIG_DOWNLOAD_WIN_HASH "3cc7dd131a87972f70fca1490b9e6e6b")

  if(WIN32)
    # swig.exe available as pre-built binary on Windows:
    ExternalProject_Add(Swig
      URL http://midas3.kitware.com/midas/api/rest?method=midas.bitstream.download&checksum=${SWIG_DOWNLOAD_WIN_HASH}&name=swigwin-${SWIG_TARGET_VERSION}.zip
      URL_MD5 ${SWIG_DOWNLOAD_WIN_HASH}
      SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}"
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      UPDATE_COMMAND ""
      )

    set(SWIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}") # ??
    set(SWIG_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}/swig.exe")
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
    find_package(BISON)
    set(BISON_FLAGS "" CACHE STRING "Flags used by bison")
    mark_as_advanced(BISON_FLAGS)

    # follow the standard EP_PREFIX locations
    set(swig_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/Swig-prefix/src/Swig-build)
    set(swig_source_dir ${CMAKE_CURRENT_BINARY_DIR}/Swig-prefix/src/Swig)
    set(swig_install_dir ${CMAKE_CURRENT_BINARY_DIR}/Swig)

    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild/swig_configure_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/swig_configure_step.cmake
      @ONLY)
    set ( swig_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/swig_configure_step.cmake )

    ExternalProject_add(Swig
      URL http://midas3.kitware.com/midas/api/rest?method=midas.bitstream.download&checksum=${SWIG_DOWNLOAD_SOURCE_HASH}&name=swig-${SWIG_TARGET_VERSION}.tar.gz
      URL_MD5 ${SWIG_DOWNLOAD_SOURCE_HASH}
      CONFIGURE_COMMAND ${swig_CONFIGURE_COMMAND}
      DEPENDS ${${proj}_DEPENDENCIES}
      )

    set(SWIG_DIR "${swig_install_dir}/share/swig/${SWIG_TARGET_VERSION}")
    set(SWIG_EXECUTABLE ${swig_install_dir}/bin/swig)
    set(Swig_DEPEND Swig)
  endif()
endif()
