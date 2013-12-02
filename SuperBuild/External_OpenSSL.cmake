
superbuild_include_once()

# Set dependency list
set(OpenSSL_DEPENDENCIES "")
if(UNIX)
  set(OpenSSL_DEPENDENCIES zlib)
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(OpenSSL)
set(proj OpenSSL)
#message(STATUS "${__indent}Adding project ${proj}")

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(OPENSSL_INCLUDE_DIR CACHE)
  if(UNIX)
    unset(OPENSSL_CRYPTO_LIBRARY CACHE)
    unset(OPENSSL_SSL_LIBRARY CACHE)
  elseif(WIN32)
    unset(LIB_EAY_DEBUG CACHE)
    unset(LIB_EAY_RELEASE CACHE)
    unset(SSL_EAY_DEBUG CACHE)
    unset(SSL_EAY_RELEASE CACHE)
  endif()
  find_package(OpenSSL REQUIRED)
endif()

if((NOT DEFINED OPENSSL_INCLUDE_DIR
  OR NOT DEFINED OPENSSL_LIBRARIES) AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  if(UNIX)
    set(OpenSSL_URL http://www.openssl.org/source/openssl-1.0.1e.tar.gz)
    set(OpenSSL_MD5 66bf6f10f060d561929de96f9dfe5b8c)

    #------------------------------------------------------------------------------
    set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

    configure_file(
      SuperBuild/${proj}_patch_and_configure_step.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/${proj}_patch_and_configure_step.cmake @ONLY)

    #------------------------------------------------------------------------------
    ExternalProject_Add(${proj}
      URL ${OpenSSL_URL}
      URL_MD5 ${OpenSSL_MD5}
      DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
      SOURCE_DIR ${EP_SOURCE_DIR}
      BUILD_IN_SOURCE 1
      PATCH_COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_BINARY_DIR}/${proj}_patch_and_configure_step.cmake
      CONFIGURE_COMMAND ""
      BUILD_COMMAND $(MAKE) -j1 build_libs
      INSTALL_COMMAND ""
      DEPENDS
        ${OpenSSL_DEPENDENCIES}
      )

    if(APPLE)
      ExternalProject_Add_Step(${proj} fix_rpath
        COMMAND install_name_tool -id ${EP_SOURCE_DIR}/libcrypto.dylib ${EP_SOURCE_DIR}/libcrypto.dylib
        COMMAND install_name_tool
          -change /usr/local/ssl/lib/libcrypto.1.0.0.dylib ${EP_SOURCE_DIR}/libcrypto.dylib
          -id ${EP_SOURCE_DIR}/libssl.dylib ${EP_SOURCE_DIR}/libssl.dylib
        DEPENDEES build
        )
    endif()

    set(OpenSSL_DIR ${EP_SOURCE_DIR})
    set(OPENSSL_INCLUDE_DIR ${OpenSSL_DIR}/include)
    set(OPENSSL_LIBRARY_DIR ${OpenSSL_DIR})
    set(OPENSSL_EXPORT_LIBRARY_DIR ${OpenSSL_DIR})

    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_LIBRARY_DIR}/libcrypto${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_LIBRARY_DIR}/libssl${CMAKE_SHARED_LIBRARY_SUFFIX})

    set(OPENSSL_LIBRARIES ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})

  elseif(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(OpenSSL_URL http://packages.kitware.com/download/item/3877/OpenSSL_1_0_1e-install-32.tar.gz)
      set(OpenSSL_MD5 aedd620319a0d3c87b03a92e2fad8f96)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(OpenSSL_URL http://packages.kitware.com/download/item/3876/OpenSSL_1_0_1e-install-64.tar.gz)
      set(OpenSSL_MD5 d57a52c20253723c17bf39594a0ebb96)
    endif()

    #------------------------------------------------------------------------------
    set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

    ExternalProject_Add(${proj}
      URL ${OpenSSL_URL}
      URL_MD5 ${OpenSSL_MD5}
      DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
      SOURCE_DIR ${EP_SOURCE_DIR}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS
        ${OpenSSL_DEPENDENCIES}
      )

    set(OpenSSL_DIR ${EP_SOURCE_DIR})
    if(DEFINED CMAKE_CONFIGURATION_TYPES)
      set(OpenSSL_DIR ${OpenSSL_DIR}/${CMAKE_CFG_INTDIR})
    endif()

    set(OPENSSL_INCLUDE_DIR "${OpenSSL_DIR}/include")
    set(OPENSSL_LIBRARY_DIR "${OpenSSL_DIR}/lib")
    set(OPENSSL_EXPORT_LIBRARY_DIR "${OpenSSL_DIR}/bin")
    set(OPENSSL_LIBRARIES "${OPENSSL_LIBRARY_DIR}/libeay32.lib" "${OPENSSL_LIBRARY_DIR}/ssleay32.lib")

    set(LIB_EAY_DEBUG "${EP_SOURCE_DIR}/Debug/lib/libeay32.lib")
    set(LIB_EAY_RELEASE "${EP_SOURCE_DIR}/Release/lib/libeay32.lib")
    set(SSL_EAY_DEBUG "${EP_SOURCE_DIR}/Debug/lib/ssleay32.lib")
    set(SSL_EAY_RELEASE "${EP_SOURCE_DIR}/Release/lib/ssleay32.lib")
  endif()

  message(STATUS "${__${proj}_superbuild_message} - OPENSSL_LIBRARY_DIR:${OPENSSL_LIBRARY_DIR}")
  message(STATUS "${__${proj}_superbuild_message} - OPENSSL_EXPORT_LIBRARY_DIR:${OPENSSL_EXPORT_LIBRARY_DIR}")
else()
  SlicerMacroEmptyExternalProject(${proj} "${OpenSSL_DEPENDENCIES}")
endif()

message(STATUS "${__${proj}_superbuild_message} - OPENSSL_INCLUDE_DIR:${OPENSSL_INCLUDE_DIR}")
message(STATUS "${__${proj}_superbuild_message} - OPENSSL_LIBRARIES:${OPENSSL_LIBRARIES}")

list_to_string(${ep_list_separator} "${OPENSSL_LIBRARIES}" OPENSSL_LIBRARIES)
