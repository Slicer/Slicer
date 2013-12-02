
superbuild_include_once()

# Set dependency list
set(curl_DEPENDENCIES zlib)
if(CURL_ENABLE_SSL)
  if(NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_curl)
    list(APPEND curl_DEPENDENCIES OpenSSL)
  else()
    # XXX - Add a test checking if system curl support OpenSSL
  endif()
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(curl)
set(proj curl)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(CURL_INCLUDE_DIR CACHE)
  unset(CURL_LIBRARY CACHE)
  find_package(CURL REQUIRED)
endif()

if((NOT DEFINED CURL_INCLUDE_DIRS
   OR NOT DEFINED CURL_LIBRARIES) AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  set(EXTERNAL_PROJECT_OPTIONAL_ARGS)

  # Set CMake OSX variable to pass down the external project
  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  if(NOT CMAKE_CONFIGURATION_TYPES)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
  endif()

  if(CURL_ENABLE_SSL)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_USE_OPENSSL:BOOL=ON
      -DOPENSSL_INCLUDE_DIR:PATH=${OPENSSL_INCLUDE_DIR}
      )
    if(UNIX)
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
        -DOPENSSL_SSL_LIBRARY:STRING=${OPENSSL_SSL_LIBRARY}
        -DOPENSSL_CRYPTO_LIBRARY:STRING=${OPENSSL_CRYPTO_LIBRARY}
        )
    elseif(WIN32)
      list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
        -DLIB_EAY_DEBUG:FILEPATH=${LIB_EAY_DEBUG}
        -DLIB_EAY_RELEASE:FILEPATH=${LIB_EAY_RELEASE}
        -DSSL_EAY_DEBUG:FILEPATH=${SSL_EAY_DEBUG}
        -DSSL_EAY_RELEASE:FILEPATH=${SSL_EAY_RELEASE}
        )
    endif()
  else()
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_USE_OPENSSL:BOOL=OFF
      )
  endif()

  set(${proj}_CMAKE_C_FLAGS ${ep_common_c_flags})
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(${proj}_CMAKE_C_FLAGS "${ep_common_c_flags} -fPIC")
  endif()

  set(EP_BUILD_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/Slicer/curl.git"
    GIT_TAG "c2bc1187192ea9565f16db6382abc574114af193"
    SOURCE_DIR curl
    BINARY_DIR ${EP_BUILD_DIR}
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
    #Not needed -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    #Not needed -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${${proj}_CMAKE_C_FLAGS}
      -DCMAKE_INSTALL_PREFIX:PATH=${EP_INSTALL_DIR}
      -DBUILD_CURL_TESTS:BOOL=OFF # BUILD_TESTING is not used
      -DBUILD_CURL_EXE:BOOL=OFF
      -DBUILD_DASHBOARD_REPORTS:BOOL=OFF
      -DCURL_STATICLIB:BOOL=ON
      -DCURL_USE_ARES:BOOL=OFF
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
      -DHAVE_LIBIDN:BOOL=FALSE
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    DEPENDS
      ${curl_DEPENDENCIES}
    )

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
  SlicerMacroEmptyExternalProject(${proj} "${curl_DEPENDENCIES}")
endif()

message(STATUS "${__${proj}_superbuild_message} - CURL_INCLUDE_DIR:${CURL_INCLUDE_DIR}")
message(STATUS "${__${proj}_superbuild_message} - CURL_LIBRARY:${CURL_LIBRARY}")
