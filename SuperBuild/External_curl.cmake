
set(proj curl)

# Set dependency list
set(${proj}_DEPENDENCIES zlib)
if(CURL_ENABLE_SSL)
  if(NOT Slicer_USE_SYSTEM_${proj})
    list(APPEND ${proj}_DEPENDENCIES OpenSSL)
  else()
    # XXX - Add a test checking if system curl support OpenSSL
  endif()
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(CURL_INCLUDE_DIR CACHE)
  unset(CURL_LIBRARY CACHE)
  find_package(CURL REQUIRED)
endif()

if((NOT DEFINED CURL_INCLUDE_DIR
   OR NOT DEFINED CURL_LIBRARY) AND NOT Slicer_USE_SYSTEM_${proj})

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS)

  if(CURL_ENABLE_SSL)
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


  set(${proj}_CMAKE_C_FLAGS ${ep_common_c_flags})
  if(CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit
    set(${proj}_CMAKE_C_FLAGS "${ep_common_c_flags} -fPIC")
  endif()

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/Slicer/curl.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "ca5fe8e63df7faea0bfb988ef3fe58f538e6950b" # slicer-7.70.0-2020-04-29-53cdc2c
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

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
    #Not needed -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    #Not needed -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${${proj}_CMAKE_C_FLAGS}
      -DCMAKE_DEBUG_POSTFIX:STRING=
      -DCMAKE_INSTALL_PREFIX:PATH=${EP_INSTALL_DIR}
      -DCMAKE_INSTALL_LIBDIR:STRING=lib  # Override value set in GNUInstallDirs CMake module
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_CURL_EXE:BOOL=OFF
      -DBUILD_SHARED_LIBS:BOOL=OFF  # Before enabling this option, see https://github.com/Slicer/curl/commit/ca5fe8e63df7faea0bfb988ef3fe58f538e6950b
      -DENABLE_ARES:BOOL=OFF
      -DCURL_ZLIB:BOOL=ON
      -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
      -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
      -DCURL_DISABLE_FTP:BOOL=ON
      -DCURL_DISABLE_LDAP:BOOL=ON
      -DCURL_DISABLE_LDAPS:BOOL=ON
      -DCURL_DISABLE_TELNET:BOOL=ON
      -DCURL_DISABLE_DICT:BOOL=ON
      -DCURL_DISABLE_FILE:BOOL=ON
      -DCURL_DISABLE_TFTP:BOOL=ON
      -DHAVE_LIBIDN2:BOOL=FALSE
      -DCMAKE_USE_LIBSSH:BOOL=OFF
      -DCMAKE_USE_LIBSSH2:BOOL=OFF
      -DCMAKE_USE_OPENSSL:BOOL=${CURL_ENABLE_SSL}
    ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  if(UNIX)
    set(curl_IMPORT_SUFFIX .a)
    if(APPLE)
      set(curl_IMPORT_SUFFIX .a)
    endif()
  elseif(WIN32)
    set(curl_IMPORT_SUFFIX .lib)
  else()
    message(FATAL_ERROR "Unknown system !")
  endif()

  set(CURL_INCLUDE_DIR "${EP_INSTALL_DIR}/include")
  set(CURL_LIBRARY "${EP_INSTALL_DIR}/lib/libcurl${curl_IMPORT_SUFFIX}")

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS
    CURL_INCLUDE_DIR:PATH
    CURL_LIBRARY:FILEPATH
  LABELS "FIND_PACKAGE"
  )

ExternalProject_Message(${proj} "CURL_INCLUDE_DIR:${CURL_INCLUDE_DIR}")
ExternalProject_Message(${proj} "CURL_LIBRARY:${CURL_LIBRARY}")
