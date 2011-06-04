# -------------------------------------------------------------------------
# Find and install LibArchive
# -------------------------------------------------------------------------
IF(WIN32)
  INSTALL(FILES ${LibArchive_DIR}/bin/archive.dll
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Runtime
    )
ELSE()
  slicerInstallLibrary(FILE ${LibArchive_LIBRARY}
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Runtime
    )
ENDIF()

