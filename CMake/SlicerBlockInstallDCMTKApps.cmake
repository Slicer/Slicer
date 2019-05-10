# -------------------------------------------------------------------------
# Find and install DCMTK Apps
# -------------------------------------------------------------------------

set(DCMTK_Apps storescu storescp dcmdump dump2dcm img2dcm dcmdjpeg dcmqrscp dcm2xml xml2dcm dsr2html dsr2xml xml2dsr dsrdump echoscu)
set(int_dir "")
if(CMAKE_CONFIGURATION_TYPES)
  set(int_dir "Release/")
endif()
foreach(dcmtk_App ${DCMTK_Apps})
  install(PROGRAMS ${DCMTK_DIR}/bin/${int_dir}${dcmtk_App}${CMAKE_EXECUTABLE_SUFFIX}
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    COMPONENT Runtime
    )
  slicerStripInstalledLibrary(
    FILES "${Slicer_INSTALL_BIN_DIR}/${dcmtk_App}"
    COMPONENT Runtime
    )
  if(APPLE)
    # Fixes Slicer issue #3827
    set(dollar "$")
    install(CODE
      "set(app ${Slicer_INSTALL_BIN_DIR}/${dcmtk_App})
       set(appfilepath \"${dollar}ENV{DESTDIR}${dollar}{CMAKE_INSTALL_PREFIX}/${dollar}{app}\")
       message(\"CPack: - Adding rpath to ${dollar}{app}\")
       execute_process(COMMAND install_name_tool -add_rpath @loader_path/..  ${dollar}{appfilepath})"
      COMPONENT Runtime
      )
  endif()
endforeach()
