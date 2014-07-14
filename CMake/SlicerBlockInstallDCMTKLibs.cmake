# -------------------------------------------------------------------------
# Find and install DCMTK Libs
# -------------------------------------------------------------------------

find_package(DCMTK REQUIRED)
foreach(dcmtk_Lib ${DCMTK_LIBRARIES})
  if(WIN32)
    install(FILES ${DCMTK_DIR}/bin/Release/${dcmtk_Lib}.dll
      DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Runtime)
  elseif(UNIX)
    install(DIRECTORY ${DCMTK_DIR}/lib/
      DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Runtime
      FILES_MATCHING PATTERN lib${dcmtk_Lib}.so*)
  endif()
endforeach()
