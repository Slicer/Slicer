
set(proj PCRE)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED PCRE_DIR AND NOT EXISTS ${PCRE_DIR})
  message(FATAL_ERROR "PCRE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
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
    ${${proj}_EP_ARGS}
    URL http://downloads.sourceforge.net/project/pcre/pcre/8.12/pcre-8.12.tar.gz
    URL_MD5 fa69e4c5d8971544acd71d1f10d59193
    UPDATE_COMMAND "" # Disable update
    CONFIGURE_COMMAND ${pcre_CONFIGURE_COMMAND}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
endif()
