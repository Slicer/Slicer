
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

#
#  PCRE (Perl Compatible Regular Expressions)
#

# follow the standard EP_PREFIX locations
set(pcre_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/PCRE-prefix/src/PCRE-build)
set(pcre_source_dir ${CMAKE_CURRENT_BINARY_DIR}/PCRE-prefix/src/PCRE)
set(pcre_install_dir ${CMAKE_CURRENT_BINARY_DIR}/PCRE)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild/pcre_configure_step.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/pcre_configure_step.cmake
  @ONLY)
set(pcre_CONFIGURE_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/pcre_configure_step.cmake)

ExternalProject_add(PCRE
  URL http://downloads.sourceforge.net/project/pcre/pcre/8.12/pcre-8.12.tar.gz
  URL_MD5 fa69e4c5d8971544acd71d1f10d59193
  UPDATE_COMMAND "" # Disable update
  CONFIGURE_COMMAND ${pcre_CONFIGURE_COMMAND}
  )
