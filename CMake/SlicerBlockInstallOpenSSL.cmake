# -------------------------------------------------------------------------
# Find and install OpenSSL Libs
# -------------------------------------------------------------------------

foreach(library ${OPENSSL_LIBRARIES})
  get_filename_component(library_without_extension ${library} NAME_WE)
  if(WIN32)
    install(FILES ${OPENSSL_EXPORT_LIBRARY_DIR}/${library_without_extension}.dll
      DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Runtime)
  elseif(UNIX)
    slicerInstallLibrary(
      FILE ${library}
      DESTINATION ${Slicer_INSTALL_LIB_DIR}
      COMPONENT Runtime
      )
  endif()
endforeach()
