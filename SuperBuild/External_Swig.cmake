
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED Swig_DIR AND NOT EXISTS ${Swig_DIR})
  message(FATAL_ERROR "Swig_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT SWIG_DIR)

  set(SWIG_TARGET_VERSION 2.0.9)
  set(SWIG_DOWNLOAD_SOURCE_HASH "54d534b14a70badc226129159412ea85")
  set(SWIG_DOWNLOAD_SOURCE_ID "147969")
  set(SWIG_DOWNLOAD_WIN_HASH "a1dc34766cf599f49e2092f7973c85f4" )
  set(SWIG_DOWNLOAD_WIN_ID "147968a")

  if(WIN32)
    # swig.exe available as pre-built binary on Windows:
    ExternalProject_Add(Swig
      URL  http://midas3.kitware.com/midas/api/rest?method=midas.bitstream.download&id=${SWIG_DOWNLOAD_WIN_ID}&name=swigwin-${SWIG_TARGET_VERSION}.tar.gz
      URL_MD5 ${SWIG_DOWNLOAD_WIN_HASH}
      SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}"
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      UPDATE_COMMAND ""
      )

    set(SWIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}") # ??
    set(SWIG_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/swigwin-${SWIG_TARGET_VERSION}"/swig.exe)
    set(Swig_DEPEND Swig)

  else()
    # not windows

    # Set dependency list
    set(Swig_DEPENDENCIES "PCRE")

    # Include dependent projects if any
    SlicerMacroCheckExternalProjectDependency(Swig)

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
      URL  http://midas3.kitware.com/midas/api/rest?method=midas.bitstream.download&id=${SWIG_DOWNLOAD_SOURCE_ID}&name=swig-${SWIG_TARGET_VERSION}.tar.gz
      URL_MD5 ${SWIG_DOWNLOAD_SOURCE_HASH}
      CONFIGURE_COMMAND ${swig_CONFIGURE_COMMAND}
      DEPENDS "${Swig_DEPENDENCIES}"
      )

    set(SWIG_DIR "${swig_install_dir}/share/swig/${SWIG_TARGET_VERSION}")
    set(SWIG_EXECUTABLE ${swig_install_dir}/bin/swig)
    set(Swig_DEPEND Swig)
  endif()
endif()
