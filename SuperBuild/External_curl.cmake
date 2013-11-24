
slicer_include_once()

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

  set(EP_BUILD_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

  ExternalProject_Add(${proj}
    URL "http://curl.haxx.se/download/curl-7.33.0.tar.gz"
    URL_MD5 c8a4eaac7ce7b0d1bf458d62ccd4ef93
    SOURCE_DIR curl
    BINARY_DIR ${EP_BUILD_DIR}
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
    #Not needed -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    #Not needed -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DBUILD_CURL_TESTS:BOOL=OFF # BUILD_TESTING is not used
      -DBUILD_CURL_EXE:BOOL=OFF
      -DBUILD_DASHBOARD_REPORTS:BOOL=OFF
      -DCURL_STATICLIB:BOOL=OFF
      -DCURL_USE_ARES:BOOL=OFF
      -DCURL_ZLIB:BOOL=ON
      -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
      -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${curl_DEPENDENCIES}
    )

  set(curl_DIR ${EP_BUILD_DIR})

  set(curl_LIBRARY_DIR ${curl_DIR}/lib)
  if(DEFINED CMAKE_CONFIGURATION_TYPES)
    set(curl_LIBRARY_DIR ${curl_DIR}/${CMAKE_CFG_INTDIR}/lib)
  endif()

  if(UNIX)
    set(curl_IMPORT_SUFFIX .so)
    if(APPLE)
      set(curl_IMPORT_SUFFIX .dylib)
    endif()
  elseif(WIN32)
    set(curl_IMPORT_SUFFIX _imp.lib)
  else()
    message(FATAL_ERROR "Unknown system !")
  endif()

  set(CURL_INCLUDE_DIR "${curl_DIR}/include")
  set(CURL_LIBRARY "${curl_LIBRARY_DIR}/libcurl${curl_IMPORT_SUFFIX}")

else()
  SlicerMacroEmptyExternalProject(${proj} "${curl_DEPENDENCIES}")
endif()

message(STATUS "${__${proj}_superbuild_message} - CURL_INCLUDE_DIR:${CURL_INCLUDE_DIR}")
message(STATUS "${__${proj}_superbuild_message} - CURL_LIBRARY:${CURL_LIBRARY}")
