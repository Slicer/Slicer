
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

set( TARGET_SWIG_VERSION 2.0.9 )
if(NOT SWIG_DIR)
  if(WIN32)
    # swig.exe available as pre-built binary on Windows:
    ExternalProject_Add(Swig
      URL http://prdownloads.sourceforge.net/swig/swigwin-${TARGET_SWIG_VERSION}.zip
      URL_MD5 a1dc34766cf599f49e2092f7973c85f4
      SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${TARGET_SWIG_VERSION}
      "${${PROJECT_NAME}_EP_DISABLED_UPDATE}"
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      )

    set(SWIG_DIR ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${TARGET_SWIG_VERSION}) # ??
    set(SWIG_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${TARGET_SWIG_VERSION}/swig.exe)
    set(Swig_DEPEND Swig)
  else()
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
      URL http://prdownloads.sourceforge.net/swig/swig-${TARGET_SWIG_VERSION}.tar.gz
      URL_MD5  54d534b14a70badc226129159412ea85
      CONFIGURE_COMMAND ${swig_CONFIGURE_COMMAND}
      DEPENDS PCRE
      )

    set(SWIG_DIR ${swig_install_dir}/share/swig/${TARGET_SWIG_VERSION})
    set(SWIG_EXECUTABLE ${swig_install_dir}/bin/swig)
    set(Swig_DEPEND Swig)
  endif()
endif()
