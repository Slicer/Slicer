#-----------------------------------------------------------------------------
macro(slicer3_set_default_modules_install_prefix)
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(Slicer3_INSTALL_PREFIX)
      set(__install_prefix "${Slicer3_INSTALL_PREFIX}")
    else(Slicer3_INSTALL_PREFIX)
      set(__install_prefix "${Slicer3_HOME}")
    endif(Slicer3_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${__install_prefix}"
      CACHE PATH "Install path prefix." FORCE)
  endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endmacro(slicer3_set_default_modules_install_prefix)

#-----------------------------------------------------------------------------
macro(slicer3_set_default_modules_output_path)
#  set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin"
#    CACHE PATH "Single output directory for building all libraries." FORCE)
#  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin"
#    CACHE PATH "Single output directory for building all executables." FORCE)
  set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_LIB_DIR}")
  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_BIN_DIR}")
  mark_as_advanced(
    LIBRARY_OUTPUT_PATH
    EXECUTABLE_OUTPUT_PATH
    )
endmacro(slicer3_set_default_modules_output_path)
