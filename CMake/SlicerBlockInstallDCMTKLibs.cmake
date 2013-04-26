# -------------------------------------------------------------------------
# Find and install DCMTK Libs
# -------------------------------------------------------------------------

foreach(dcmtk_Lib ${DCMTK_LIBRARIES})
  if(WIN32)
    install(FILES ${DCMTK_DIR}/bin/Release/${dcmtk_Lib}.dll
      DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Runtime)
  elseif(APPLE)
    # needs to install symlink version named libraries as well.
    install(FILES ${DCMTK_DIR}/lib/lib${dcmtk_Lib}.dylib
      DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Runtime)
  elseif(UNIX)
    install(FILES ${DCMTK_DIR}/lib/lib${dcmtk_Lib}.so
      DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Runtime)
  endif()
endforeach()
