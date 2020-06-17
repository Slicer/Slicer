# -------------------------------------------------------------------------
# Find and install OpenSSL Libs
# -------------------------------------------------------------------------

find_package(OpenSSL REQUIRED) # Needed to set OPENSSL_VERSION
set(library_suffix "")
if(WIN32 AND OPENSSL_VERSION VERSION_GREATER_EQUAL "1.1.0")
  # Starting with OpenSSL 1.1.0, shared libraries are named
  # * libcrypto-1_1.dll and libssl-1_1.dll for 32-bit
  # * libcrypto-1_1-x64.dll and libssl-1_1-x64.dll for 64-bit

  string(REGEX MATCHALL "[0-9]+" openssl_versions "${OPENSSL_VERSION}")
  list(GET openssl_versions 0 openssl_version_major)
  list(GET openssl_versions 1 openssl_version_minor)

  set(library_suffix "-${openssl_version_major}_${openssl_version_minor}")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit
    string(APPEND library_suffix "-x64")
  endif()
endif()

set(library_config "general")
foreach(library ${OPENSSL_LIBRARIES})
  if(library MATCHES "debug|general|optimized")
    set(library_config ${library})
  else()
    if(library_config MATCHES "general|optimized")
      if(WIN32)
        get_filename_component(library_without_extension ${library} NAME_WE)
        install(FILES ${OPENSSL_EXPORT_LIBRARY_DIR}/${library_without_extension}${library_suffix}.dll
          DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT Runtime)
      elseif(UNIX)
        slicerInstallLibrary(
          FILE ${library}
          DESTINATION ${Slicer_INSTALL_LIB_DIR}
          COMPONENT Runtime
          STRIP
          )
      endif()
    endif()
  endif()
endforeach()
