# -------------------------------------------------------------------------
# Find and install OpenSSL Libs
# -------------------------------------------------------------------------

set(library_config "general")
foreach(library ${OPENSSL_LIBRARIES})
  if(library MATCHES "debug|general|optimized")
    set(library_config ${library})
  else()
    if(library_config MATCHES "general|optimized")
      if(WIN32)
        get_filename_component(library_without_extension ${library} NAME_WE)
        install(FILES ${OPENSSL_EXPORT_LIBRARY_DIR}/${library_without_extension}.dll
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
