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
  )
