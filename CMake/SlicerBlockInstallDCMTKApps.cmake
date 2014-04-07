# -------------------------------------------------------------------------
# Find and install DCMTK Apps
# -------------------------------------------------------------------------

set(DCMTK_Apps storescu storescp dcmdump dump2dcm img2dcm dcmdjpeg dcmqrscp)
set(int_dir "")
if(CMAKE_CONFIGURATION_TYPES)
  set(int_dir "Release/")
endif()
foreach(dcmtk_App ${DCMTK_Apps})
  install(PROGRAMS ${CTK_DCMTK_DIR}/bin/${int_dir}${dcmtk_App}${CMAKE_EXECUTABLE_SUFFIX}
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    COMPONENT Runtime
    )
endforeach()
