# -------------------------------------------------------------------------
# Find and install PythonQt
# -------------------------------------------------------------------------
set(PYTHONQT_INSTALL_LIB_DIR "${Slicer_INSTALL_LIB_DIR}")

if(NOT EXISTS "${PYTHONQT_INSTALL_DIR}")
  message(FATAL_ERROR "CMake variable PYTHONQT_INSTALL_DIR is set to a nonexistent directory: ${PYTHONQT_INSTALL_DIR}")
endif()

if(WIN32)
  install(FILES ${PYTHONQT_INSTALL_DIR}/bin/PythonQt.dll
    DESTINATION bin COMPONENT Runtime)
elseif(APPLE)
  # needs to install symlink version named libraries as well.
  install(FILES ${PYTHONQT_INSTALL_DIR}/lib/libPythonQt.dylib
    DESTINATION ${PYTHONQT_INSTALL_LIB_DIR} COMPONENT Runtime)
elseif(UNIX)
  install(FILES ${PYTHONQT_INSTALL_DIR}/lib/libPythonQt.so
    DESTINATION ${PYTHONQT_INSTALL_LIB_DIR} COMPONENT Runtime)
  slicerStripInstalledLibrary(
    FILES "${PYTHONQT_INSTALL_LIB_DIR}/libPythonQt.so"
    COMPONENT Runtime)
endif()

