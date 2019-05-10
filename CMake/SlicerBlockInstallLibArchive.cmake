# -------------------------------------------------------------------------
# Find and install LibArchive
# -------------------------------------------------------------------------
if(WIN32)
  install(FILES ${LibArchive_DIR}/bin/archive.dll
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Runtime
    )
else()
  slicerInstallLibrary(FILE ${LibArchive_LIBRARY}
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Runtime
    STRIP
    )
endif()

