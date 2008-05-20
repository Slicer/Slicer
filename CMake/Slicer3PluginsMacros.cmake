#-----------------------------------------------------------------------------
macro(slicer3_set_default_plugins_output_path)
#  set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}"
#    CACHE PATH "Single output directory for building all libraries." FORCE)
#  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}"
#    CACHE PATH "Single output directory for building all executables." FORCE)
  set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}")
  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}")
  mark_as_advanced(
    LIBRARY_OUTPUT_PATH
    EXECUTABLE_OUTPUT_PATH
    )
endmacro(slicer3_set_default_plugins_output_path)
