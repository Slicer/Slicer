
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

set( TARGET_SWIG_VERSION 2.0.4 )
if(NOT SWIG_DIR)
  if(WIN32)
    # swig.exe available as pre-built binary on Windows:
    ExternalProject_Add(Swig
      URL http://prdownloads.sourceforge.net/swig/swigwin-${TARGET_SWIG_VERSION}.zip
      URL_MD5 4ab8064b1a8894c8577ef9d0fb2523c8
      SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/swigwin-${TARGET_SWIG_VERSION}
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

    ExternalProject_add(Swig
      URL http://prdownloads.sourceforge.net/swig/swig-${TARGET_SWIG_VERSION}.tar.gz
      URL_MD5  4319c503ee3a13d2a53be9d828c3adc0
      CONFIGURE_COMMAND ../Swig/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}
      --with-pcre-prefix=${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS PCRE
      )

    set(SWIG_DIR ${CMAKE_CURRENT_BINARY_DIR}/share/swig/${TARGET_SWIG_VERSION})
    set(SWIG_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/bin/swig)
    set(Swig_DEPEND Swig)
  endif()
endif(NOT SWIG_DIR)


