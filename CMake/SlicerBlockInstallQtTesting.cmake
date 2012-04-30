# -------------------------------------------------------------------------
# Find and install QtTesting
# -------------------------------------------------------------------------

set(QtTesting_INSTALL_LIB_DIR "${Slicer_INSTALL_LIB_DIR}")

if(WIN32)
  install(FILES ${QtTesting_INSTALL_DIR}/bin/QtTesting.dll
    DESTINATION bin COMPONENT Runtime)
elseif(APPLE)
  # needs to install symlink version named libraries as well.
  install(FILES ${QtTesting_INSTALL_DIR}/lib/libQtTesting.dylib
    DESTINATION ${QtTesting_INSTALL_LIB_DIR} COMPONENT Runtime)
elseif(UNIX)
  install(FILES ${QtTesting_INSTALL_DIR}/lib/libQtTesting.so
    DESTINATION ${QtTesting_INSTALL_LIB_DIR} COMPONENT Runtime)
endif()

