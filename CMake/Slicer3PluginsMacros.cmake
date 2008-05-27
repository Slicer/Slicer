#-----------------------------------------------------------------------------
# Set the default output paths for one or more plugins/CLP
# 
macro(slicer3_set_plugins_output_path)
  set_target_properties(${ARGN}
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
    )
endmacro(slicer3_set_plugins_output_path)

#-----------------------------------------------------------------------------
# Install one or more plugins to the default plugin location
# 
macro(slicer3_install_plugins)
  install(TARGETS ${ARGN}
    RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
    )
endmacro(slicer3_install_plugins)
