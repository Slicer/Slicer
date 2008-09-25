# -------------------------------------------------------------------------
# Find and install Tcl

if(TCL_INCLUDE_PATH)
  get_filename_component(TCL_DIR "${TCL_INCLUDE_PATH}" PATH)
else(TCL_INCLUDE_PATH)
  set(TCL_DIR "${Slicer3_BINARY_DIR}/../Slicer3-lib/tcl-build")
endif(TCL_INCLUDE_PATH)

if(NOT EXISTS "${TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT}" AND 
    NOT EXISTS "${TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe")
  message(STATUS "Not Packaging TCL (cannot found: ${TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT} or ${TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe)")
  set(TCL_DIR "")
endif(NOT EXISTS "${TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT}" AND 
  NOT EXISTS "${TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe")

# Note: this is probably dangerous if Tcl is somewhere in /usr/local, as it
# ends up installing the whole /usr/local to the Slicer3 install tree :(
# TODO: use VTK/KWWidgets macros to copy only the files that are known to
# belong to Tcl/Tk; in the meantime only a few people are using external
# VTK/KWWidgets/Teem/TclTk packages, so we will assume they know what they
#  are doing (i.e. they have Tcl/Tk installed in a standalone directory 
# like /opt/tcltk8.5.0)

if(TCL_DIR)
  install(DIRECTORY
    ${TCL_DIR}/
    DESTINATION lib/TclTk
    )
endif(TCL_DIR)

# -------------------------------------------------------------------------
# Find and install python

if(Slicer3_USE_PYTHON)
  set(PYTHON_DIR "${Slicer3_BINARY_DIR}/../Slicer3-lib/python-build")
  if(WIN32 AND NOT EXISTS "${PYTHON_DIR}" AND EXISTS "${PYTHON_INCLUDE_PATH}")
    get_filename_component(PYTHON_DIR "${PYTHON_INCLUDE_PATH}" PATH)
  endif(WIN32 AND NOT EXISTS "${PYTHON_DIR}" AND EXISTS "${PYTHON_INCLUDE_PATH}")
  if(EXISTS "${PYTHON_DIR}")
    install(DIRECTORY
      "${PYTHON_DIR}/"
      DESTINATION lib/Python
      )
  endif(EXISTS "${PYTHON_DIR}")
endif(Slicer3_USE_PYTHON)

# -------------------------------------------------------------------------
# CPack

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in"
  "${Slicer3_BINARY_DIR}/PackageLibraries/CMakeLists.txt"
  @ONLY)

include(InstallRequiredSystemLibraries)
include(${CMAKE_CURRENT_SOURCE_DIR}/InstallDebugSystemLibraries.cmake)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Medical Visualization and Processing Environment for Research")
set(CPACK_PACKAGE_VENDOR "NA-MIC")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${Slicer3_SOURCE_DIR}/README.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${Slicer3_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Slicer3_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Slicer3_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${Slicer3_VERSION_PATCH}")
set(CPACK_INSTALL_CMAKE_PROJECTS
  "${Slicer3_BINARY_DIR};Slicer3;ALL;/")
if(EXISTS "${VTK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${VTK_DIR};VTK;ALL;/")
endif(EXISTS "${VTK_DIR}/CMakeCache.txt")
if(EXISTS "${ITK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;ALL;/")
endif(EXISTS "${ITK_DIR}/CMakeCache.txt")
if(EXISTS "${KWWidgets_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${KWWidgets_DIR};KWWidgets;ALL;/")
endif(EXISTS "${KWWidgets_DIR}/CMakeCache.txt")
if(EXISTS "${TEEM_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${TEEM_DIR};teem;ALL;/")
endif(EXISTS "${TEEM_DIR}/CMakeCache.txt")
if(APPLE)
  set(CPACK_PACKAGE_ICON "${Slicer3_SOURCE_DIR}/Utilities/Package/Slicer3.icns")
endif(APPLE)

# slicer3 does _not_ require setting the windows path
set(CPACK_NSIS_MODIFY_PATH OFF)

set(APPLICATION_NAME "Slicer")
set(EXECUTABLE_NAME "Slicer3")
set(CPACK_PACKAGE_EXECUTABLES "..\\\\${EXECUTABLE_NAME}" "${APPLICATION_NAME}")

# -------------------------------------------------------------------------
# File extensions

set(FILE_EXTENSIONS .mrml .xcat)

if(FILE_EXTENSIONS)

  # For NSIS (Win32) now, we will add MacOSX support later (get back to Wes)

  if(WIN32 AND NOT UNIX)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS)
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS)
    foreach(ext ${FILE_EXTENSIONS})
      string(LENGTH "${ext}" len)
      math(EXPR len_m1 "${len} - 1")
      string(SUBSTRING "${ext}" 1 ${len_m1} ext_wo_dot)
      set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS 
        "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
            WriteRegStr HKCR \\\"${APPLICATION_NAME}\\\" \\\"\\\" \\\"${APPLICATION_NAME} supported file\\\"
            WriteRegStr HKCR \\\"${APPLICATION_NAME}\\\\shell\\\\open\\\\command\\\" \\\"\\\" \\\"$\\\\\\\"$INSTDIR\\\\${EXECUTABLE_NAME}.exe$\\\\\\\" $\\\\\\\"%1$\\\\\\\"\\\"
            WriteRegStr HKCR \\\"${ext}\\\" \\\"\\\" \\\"${APPLICATION_NAME}\\\"
            WriteRegStr HKCR \\\"${ext}\\\" \\\"Content Type\\\" \\\"application/x-${ext_wo_dot}\\\"
          ")
      set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
            DeleteRegKey HKCR \\\" ${APPLICATION_NAME}\\\"
            DeleteRegKey HKCR \\\"${ext}\\\"
          ")
    endforeach(ext)
  endif(WIN32 AND NOT UNIX)

endif(FILE_EXTENSIONS)

include(CPack)

