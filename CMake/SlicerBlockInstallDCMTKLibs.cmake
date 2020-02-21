# -------------------------------------------------------------------------
# Find and install DCMTK Libs
# -------------------------------------------------------------------------

find_package(DCMTK REQUIRED)
foreach(dcmtk_Lib ${DCMTK_LIBRARIES})
  if(WIN32)
    if(CMAKE_CONFIGURATION_TYPES)
      set(int_dir "Release/")
    endif()
    install(FILES ${DCMTK_DIR}/bin/${int_dir}${dcmtk_Lib}.dll
      DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Runtime)
  elseif(UNIX)
    install(DIRECTORY ${DCMTK_DIR}/lib/
      DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Runtime
      FILES_MATCHING PATTERN lib${dcmtk_Lib}.so*)
    slicerStripInstalledLibrary(
      FILES "${Slicer_INSTALL_LIB_DIR}/lib${dcmtk_Lib}.so"
      COMPONENT Runtime
      )
  endif()
endforeach()
