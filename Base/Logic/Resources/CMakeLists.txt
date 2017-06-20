# --------------------------------------------------------------------------
# Install support files
# --------------------------------------------------------------------------

# Should this location change, update Modules/Loadable/Colors/Logic/vtkSlicerColorLogic.cxx

file(GLOB COLORFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "ColorFiles/*.txt")

foreach(colorfile ${COLORFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${colorfile}
    ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${colorfile}
    COPYONLY
    )
endforeach()
install(
  FILES ${COLORFILES}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/ColorFiles
  COMPONENT Runtime
  )

set(orientation_markers_dir "OrientationMarkers")
set(orientation_marker_filenames
  Human.vtp
  )
foreach(filename ${orientation_marker_filenames})
  set(orientation_marker_filepath ${CMAKE_CURRENT_SOURCE_DIR}/${orientation_markers_dir}/${filename})
  configure_file(
    ${orientation_marker_filepath}
    ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${orientation_markers_dir}/${filename}
    COPYONLY
    )
  install(
    FILES ${orientation_marker_filepath}
    DESTINATION ${Slicer_INSTALL_SHARE_DIR}/${orientation_markers_dir}
    COMPONENT Runtime
    )
endforeach()
