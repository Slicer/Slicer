#-----------------------------------------------------------------------------
# Set the output paths for one (or more) Loadable Module(s)
# 
macro(slicer3_set_modules_output_path)
  set_target_properties(${ARGN}
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_LIB_DIR}"
    )
endmacro(slicer3_set_modules_output_path)

#-----------------------------------------------------------------------------
# Install one or more modules to the default plugin location
# 
macro(slicer3_install_modules)
  install(TARGETS ${ARGN}
    RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
    )
endmacro(slicer3_install_modules)
