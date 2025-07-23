
set(proj zlib)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(ZLIB_ROOT CACHE)
  find_package(ZLIB REQUIRED)
  set(ZLIB_INCLUDE_DIR ${ZLIB_INCLUDE_DIRS})
  set(ZLIB_LIBRARY ${ZLIB_LIBRARIES})
endif()

# Sanity checks
if(DEFINED ZLIB_ROOT AND NOT EXISTS ${ZLIB_ROOT})
  message(FATAL_ERROR "ZLIB_ROOT variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ZLIB_ROOT AND NOT Slicer_USE_SYSTEM_${proj})

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/Slicer/zlib-ng.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "de0aca6040339aad56d96ab1c29850b00ec36a9b"  # slicer-2.2.4-2025-02-10-860e4cf
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
      ## CXX should not be needed, but it a cmake default test
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      # Options
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DZLIB_SYMBOL_PREFIX:STRING=slicer_zlib_
      -DZLIB_COMPAT:BOOL=ON
      -DZLIB_ENABLE_TESTS:BOOL=OFF
      # Install directories
      -DCMAKE_INSTALL_LIBDIR:STRING=lib  # Override value set in GNUInstallDirs CMake module
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(zlib_DIR ${EP_INSTALL_DIR})
  set(ZLIB_ROOT ${zlib_DIR})
  set(ZLIB_INCLUDE_DIR ${zlib_DIR}/include)
  if(WIN32)
    set(ZLIB_LIBRARY ${zlib_DIR}/lib/zlib.lib)
  else()
    set(ZLIB_LIBRARY ${zlib_DIR}/lib/libz.a)
  endif()
else()
  # The project is provided using zlib_DIR, nevertheless since other project may depend on zlib,
  # let's add an 'empty' one
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS
    ZLIB_INCLUDE_DIR:PATH
    ZLIB_LIBRARY:FILEPATH
    ZLIB_ROOT:PATH
  LABELS "FIND_PACKAGE"
  )

ExternalProject_Message(${proj} "ZLIB_INCLUDE_DIR:${ZLIB_INCLUDE_DIR}")
ExternalProject_Message(${proj} "ZLIB_LIBRARY:${ZLIB_LIBRARY}")
if(ZLIB_ROOT)
  ExternalProject_Message(${proj} "ZLIB_ROOT:${ZLIB_ROOT}")
endif()
