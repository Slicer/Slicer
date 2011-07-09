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

  set(TclTk_INSTALL_LIB_DIR ${Slicer_INSTALL_ROOT}lib/TclTk)

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

