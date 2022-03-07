
set(proj LibFFI)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(LibFFI_INCLUDE_DIR CACHE)
  find_path(LibFFI_INCLUDE_DIR ffi.h)

  unset(LibFFI_LIBRARY CACHE)
  find_library(LibFFI_LIBRARY NAMES ffi libffi)
endif()

# Sanity checks
if(DEFINED LibFFI_INCLUDE_DIR AND NOT EXISTS ${LibFFI_INCLUDE_DIR})
  message(FATAL_ERROR "LibFFI_INCLUDE_DIR variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED LibFFI_LIBRARY AND NOT EXISTS ${LibFFI_LIBRARY})
  message(FATAL_ERROR "LibFFI_LIBRARY variable is defined but corresponds to nonexistent file")
endif()

if((NOT DEFINED LibFFI_INCLUDE_DIR
   OR NOT DEFINED LibFFI_LIBRARY) AND NOT Slicer_USE_SYSTEM_${proj})

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/python-cmake-buildsystem/libffi.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "e8599d9d5a50841c11a5ff8e3438dd12f080b096" # libffi-cmake-buildsystem-v3.4.2-2021-06-28-f9ea416
    QUIET
    )

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_DIR ${EP_INSTALL_DIR}
    CMAKE_CACHE_ARGS
      # Compiler settings
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      # Install directories
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(LibFFI_DIR ${EP_INSTALL_DIR})
  set(LibFFI_INCLUDE_DIR ${LibFFI_DIR}/include)
  if(WIN32)
    set(LibFFI_LIBRARY ${LibFFI_DIR}/lib/ffi_static.lib)
  else()
    set(LibFFI_LIBRARY ${LibFFI_DIR}/lib/libffi.a)
  endif()
else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS
    LibFFI_INCLUDE_DIR:PATH
    LibFFI_LIBRARY:FILEPATH
  LABELS "FIND_PACKAGE"
  )

ExternalProject_Message(${proj} "LibFFI_INCLUDE_DIR:${LibFFI_INCLUDE_DIR}")
ExternalProject_Message(${proj} "LibFFI_LIBRARY:${LibFFI_LIBRARY}")

