
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED PCRE_DIR AND NOT EXISTS ${PCRE_DIR})
  message(FATAL_ERROR "PCRE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(PCRE_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(PCRE)

# Perl Compatible Regular Expressions
ExternalProject_add(PCRE
  URL http://downloads.sourceforge.net/project/pcre/pcre/8.12/pcre-8.12.tar.gz
  URL_MD5 fa69e4c5d8971544acd71d1f10d59193
  CONFIGURE_COMMAND ../PCRE/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}
  BUILD_COMMAND ${BUILD_COMMAND_STRING}
  )

