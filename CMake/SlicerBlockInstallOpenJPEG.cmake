# -------------------------------------------------------------------------
# Find and install OpenJPEG
# -------------------------------------------------------------------------
if(NOT EXISTS "${OpenJPEG_INSTALL_DIR}")
  message(FATAL_ERROR "CMake variable OpenJPEG_INSTALL_DIR is set to a nonexistent directory: ${OpenJPEG_INSTALL_DIR}")
endif()

if(WIN32)
  install(FILES ${OpenJPEG_INSTALL_DIR}/bin/openjp2.dll
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Runtime
    )
else()
  slicerInstallLibrary(FILE ${OpenJPEG_INSTALL_DIR}/lib/libopenjp2${CMAKE_SHARED_LIBRARY_SUFFIX}
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Runtime
    STRIP
    )
endif()
