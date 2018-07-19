
set(proj bzip2)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(BZIP2_INCLUDE_DIR CACHE)
  unset(BZIP2_LIBRARIES CACHE)
  find_package(BZip2 REQUIRED)
endif()

# Sanity checks
if(DEFINED BZIP2_INCLUDE_DIR AND NOT EXISTS ${BZIP2_INCLUDE_DIR})
  message(FATAL_ERROR "BZIP2_INCLUDE_DIR variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED BZIP2_LIBRARIES AND NOT EXISTS ${BZIP2_LIBRARIES})
  message(FATAL_ERROR "BZIP2_LIBRARIES variable is defined but corresponds to nonexistent file")
endif()

if((NOT DEFINED BZIP2_INCLUDE_DIR
    OR NOT DEFINED BZIP2_LIBRARIES
    )
    AND NOT Slicer_USE_SYSTEM_${proj})

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/commontk/bzip2.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "0e735f23032ececcf52ed49b27928390fff28e50"
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_DIR ${EP_INSTALL_DIR}
    CMAKE_CACHE_ARGS
      #-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_DEBUG_POSTFIX:STRING=
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(BZIP2_INCLUDE_DIR ${EP_SOURCE_DIR})
  if(WIN32)
    set(BZIP2_LIBRARIES ${EP_INSTALL_DIR}/lib/libbz2.lib)
  else()
    set(BZIP2_LIBRARIES ${EP_INSTALL_DIR}/lib/libbz2.a)
  endif()
else()
  # The project is provided using zlib_DIR, nevertheless since other project may depend on zlib,
  # let's add an 'empty' one
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS
    BZIP2_INCLUDE_DIR:PATH
    BZIP2_LIBRARIES:FILEPATH
  LABELS "FIND_PACKAGE"
  )

ExternalProject_Message(${proj} "BZIP2_INCLUDE_DIR:${BZIP2_INCLUDE_DIR}")
ExternalProject_Message(${proj} "BZIP2_LIBRARIES:${BZIP2_LIBRARIES}")
