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
      DESTINATION ${Slicer_BUNDLE_LOCATION}/bin 
      PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ
      COMPONENT Runtime)
  else()
    install(FILES ${CTK_DIR}/CMakeExternals/Install/bin/${dcmtk_App}${EXE}
      DESTINATION bin 
      PERMISSIONS 
      OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ 
      COMPONENT Runtime)
  endif()
endforeach()
