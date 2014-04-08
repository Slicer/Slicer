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

    set(extra_exclude_pattern)
    if(UNIX)
      list(APPEND extra_exclude_pattern
        REGEX "/bin" EXCLUDE
        )
    endif()
    if(APPLE)
      list(APPEND extra_exclude_pattern
        REGEX "lib/libtcl${TCL_TK_VERSION_DOT}.dylib" EXCLUDE
        REGEX "lib/libtk${TCL_TK_VERSION_DOT}.dylib" EXCLUDE
        )
    endif()

    install(DIRECTORY
      ${Slicer_TCL_DIR}/
      DESTINATION ${TclTk_INSTALL_LIB_DIR}
      COMPONENT Runtime
      USE_SOURCE_PERMISSIONS
      REGEX "/man" EXCLUDE
      REGEX "/include" EXCLUDE
      REGEX "/demos" EXCLUDE
      PATTERN "*.a" EXCLUDE
      PATTERN "*.sh" EXCLUDE
      PATTERN "*.c" EXCLUDE
      PATTERN "tclsh${TCL_TK_VERSION_DOT}" EXCLUDE
      PATTERN "wish${TCL_TK_VERSION_DOT}" EXCLUDE
      PATTERN ".svn" EXCLUDE
      ${extra_exclude_pattern}
      )
  endif()
endif()

