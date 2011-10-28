
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

if(WIN32 AND NOT EXISTS ${Swig_DIR})
  message(FATAL_ERROR "Swig_DIR variable is expected to be set on Windows.")
endif()

# Set dependency list
set(Swig_DEPENDENCIES "PCRE")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(Swig)

ExternalProject_add(Swig
  URL http://prdownloads.sourceforge.net/swig/swig-2.0.1.tar.gz
  URL_MD5 df4465a62ccc5f0120fee0890ea1a31f
  CONFIGURE_COMMAND ../Swig/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}
#  --without-pcre
  --with-pcre-prefix=${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${Swig_DEPENDENCIES}
  )
set(SWIG_DIR ${CMAKE_CURRENT_BINARY_DIR}/share/swig/2.0.1)
set(SWIG_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/bin/swig)
set(Swig_DEPEND Swig)

