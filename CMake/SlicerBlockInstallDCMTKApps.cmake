# -------------------------------------------------------------------------
# Find and install DCMTK Apps
# -------------------------------------------------------------------------

if(WIN32)
  set(EXE ".exe")
endif()

set(DCMTK_Apps storescu storescp dcmdump dump2dcm img2dcm dcmdjpeg dcmqrscp)
set(int_dir "")
if(WIN32)
  set(int_dir "Release/")
endif()
foreach(dcmtk_App ${DCMTK_Apps})
  install(FILES ${CTK_DCMTK_DIR}/bin/${int_dir}${dcmtk_App}${EXE}
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ
    COMPONENT Runtime
    )
endforeach()
