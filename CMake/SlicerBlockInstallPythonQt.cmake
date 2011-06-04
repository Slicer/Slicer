# -------------------------------------------------------------------------
# Find and install PythonQt
# -------------------------------------------------------------------------
IF(Slicer_USE_PYTHONQT)

  SET(PYTHONQT_INSTALL_LIB_DIR "${Slicer_INSTALL_LIB_DIR}")
  
  IF(WIN32)
    INSTALL(FILES ${PYTHONQT_INSTALL_DIR}/bin/PythonQt.dll 
      DESTINATION bin COMPONENT Runtime)
  ELSEIF(APPLE)
    # needs to install symlink version named libraries as well.
    INSTALL(FILES ${PYTHONQT_INSTALL_DIR}/lib/libPythonQt.dylib 
      DESTINATION ${PYTHONQT_INSTALL_LIB_DIR} COMPONENT Runtime)
  ELSEIF(UNIX)
    INSTALL(FILES ${PYTHONQT_INSTALL_DIR}/lib/libPythonQt.so 
      DESTINATION ${PYTHONQT_INSTALL_LIB_DIR} COMPONENT Runtime)
  ENDIF()
ENDIF()

