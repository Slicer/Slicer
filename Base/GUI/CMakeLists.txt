project(SlicerBaseGUI)

# --------------------------------------------------------------------------
# Install support files

file(GLOB TCLFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Tcl/*.tcl")
foreach(tclfile ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${tclfile}
    ${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${tclfile}
    COPYONLY)
endforeach()

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/Tcl
  )
