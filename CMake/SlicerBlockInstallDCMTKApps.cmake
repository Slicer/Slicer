# -------------------------------------------------------------------------
# Find and install DCMTK Apps
# -------------------------------------------------------------------------

if(WIN32)
  set(EXE ".exe")
endif()

set(DCMTK_Apps storescu storescp dcmdump)
foreach(dcmtk_App ${DCMTK_Apps})
  if(APPLE)
    install(FILES ${CTK_DIR}/CMakeExternals/Install/bin/${dcmtk_App}
      DESTINATION USE_SOURCE_PERMISSIONS ${Slicer_BUNDLE_LOCATION}/bin COMPONENT Runtime)
  else()
    install(FILES ${CTK_DIR}/CMakeExternals/Install/bin/${dcmtk_App}${EXE}
        DESTINATION USE_SOURCE_PERMISSIONS bin COMPONENT Runtime)
  endif()
endforeach()
