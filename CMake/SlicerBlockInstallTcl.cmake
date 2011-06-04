# -------------------------------------------------------------------------
# Find and install Tcl
# -------------------------------------------------------------------------
if(Slicer_USE_PYTHONQT_WITH_TCL)
  if(NOT Slicer_TCL_DIR)
    get_filename_component(Slicer_TCL_DIR "${TCL_INCLUDE_PATH}" PATH)
  endif()

  if(NOT EXISTS "${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT}" AND
      NOT EXISTS "${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe")
    message(STATUS "Not Packaging TCL (cannot found: ${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT} or ${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe)")
    set(Slicer_TCL_DIR "")
  endif()

  # Note: this is probably dangerous if Tcl is somewhere in /usr/local, as it
  # ends up installing the whole /usr/local to the Slicer3 install tree :(
  # TODO: use VTK/KWWidgets macros to copy only the files that are known to
  # belong to Tcl/Tk; in the meantime only a few people are using external
  # VTK/KWWidgets/Teem/TclTk packages, so we will assume they know what they
  #  are doing (i.e. they have Tcl/Tk installed in a standalone directory 
  # like /opt/tcltk8.5.0)
  
  SET(TclTk_INSTALL_LIB_DIR lib/TclTk)
  
  if(Slicer_TCL_DIR)
    install(DIRECTORY
      ${Slicer_TCL_DIR}/
      DESTINATION ${TclTk_INSTALL_LIB_DIR}
      USE_SOURCE_PERMISSIONS
      REGEX "man/" EXCLUDE
      REGEX "include/" EXCLUDE
      REGEX "demos/" EXCLUDE
      PATTERN "*.sh" EXCLUDE
      PATTERN "*.c" EXCLUDE
      PATTERN "tclsh${TCL_TK_VERSION_DOT}" EXCLUDE
      PATTERN "wish${TCL_TK_VERSION_DOT}" EXCLUDE
      )
  endif()
endif()

