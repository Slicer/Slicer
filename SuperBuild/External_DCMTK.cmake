
set(proj DCMTK)

# Set dependency list
set(${proj}_DEPENDENCIES "zlib")
if(DCMTK_WITH_OPENSSL)
  if(NOT Slicer_USE_SYSTEM_${proj})
    list(APPEND ${proj}_DEPENDENCIES OpenSSL)
  else()
    # XXX - Add a test checking if system DCMTK supports OpenSSL
  endif()
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(DCMTK_DIR CACHE)
  find_package(DCMTK REQUIRED)
endif()

# Sanity checks
if(DEFINED DCMTK_DIR AND NOT EXISTS ${DCMTK_DIR})
  message(FATAL_ERROR "DCMTK_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED DCMTK_DIR AND NOT Slicer_USE_SYSTEM_${proj})
  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS)
  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)

  if(DCMTK_WITH_OPENSSL)
    # Required by "FindOpenSSL.cmake" module provided by CMake
    # and used in "DCMTK/CMake/3rdparty.cmake" when DCMTK_USE_FIND_PACKAGE is ON
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
      -DOPENSSL_INCLUDE_DIR:PATH=${OPENSSL_INCLUDE_DIR}
      )
    if(UNIX)
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
        -DOPENSSL_SSL_LIBRARY:STRING=${OPENSSL_SSL_LIBRARY}
        -DOPENSSL_CRYPTO_LIBRARY:STRING=${OPENSSL_CRYPTO_LIBRARY}
        )
    elseif(WIN32)
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
        -DLIB_EAY_DEBUG:FILEPATH=${LIB_EAY_DEBUG}
        -DLIB_EAY_RELEASE:FILEPATH=${LIB_EAY_RELEASE}
        -DSSL_EAY_DEBUG:FILEPATH=${SSL_EAY_DEBUG}
        -DSSL_EAY_RELEASE:FILEPATH=${SSL_EAY_RELEASE}
        )
    endif()
  endif()

  if(CTEST_USE_LAUNCHERS)
    set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DCMAKE_PROJECT_DCMTK_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake
      )
  endif()

  if(UNIX)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DDCMTK_FORCE_FPIC_ON_UNIX:BOOL=ON
      -DDCMTK_WITH_WRAP:BOOL=OFF   # CTK does not build on Mac with this option turned ON due to library dependencies missing
      )
  endif()

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/DCMTK/DCMTK.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "a7369385d91f40e19a9b2d4ef922c61370dfc5b1" # DCMTK 3.7.0++ 2026-01-22
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

  # If it applies, prepend "CMAKE_ARGS"
  if(NOT EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS STREQUAL "")
    set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
      CMAKE_ARGS
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS})
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DDCMTK_WITH_DOXYGEN:BOOL=OFF
      -DDCMTK_USE_FIND_PACKAGE:BOOL=ON
      -DDCMTK_WITH_ZLIB:BOOL=OFF # see CTK github issue #25
      -DDCMTK_WITH_OPENSSL:BOOL=${DCMTK_WITH_OPENSSL}
      -DDCMTK_WITH_PNG:BOOL=OFF # see CTK github issue #25
      -DDCMTK_WITH_TIFF:BOOL=OFF  # see CTK github issue #25
      -DDCMTK_WITH_XML:BOOL=OFF  # see CTK github issue #25
      -DDCMTK_WITH_ICONV:BOOL=OFF  # see CTK github issue #178
      -DDCMTK_WITH_SNDFILE:BOOL=OFF # see DCMQI github issue #395
      -DDCMTK_OVERWRITE_WIN32_COMPILER_FLAGS:BOOL=OFF
      -DDCMTK_DEFAULT_DICT:STRING=builtin
      -DDCMTK_ENABLE_PRIVATE_TAGS:BOOL=ON
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
  )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(DCMTK_DIR ${EP_BINARY_DIR})

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(_lib_subdir lib)
  if(WIN32)
    set(_lib_subdir bin)
  endif()

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${DCMTK_DIR}/${_lib_subdir}/<CMAKE_CFG_INTDIR>)
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS DCMTK_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
