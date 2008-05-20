#SET(CPACK_INSTALLED_DIRECTORIES)

set(TCL_DIR "${Slicer3_BINARY_DIR}/../Slicer3-lib/tcl-build")
if(EXISTS "${TCL_DIR}/bin/tclsh8.4" OR EXISTS "${TCL_DIR}/bin/tclsh84.exe")
else(EXISTS "${TCL_DIR}/bin/tclsh8.4" OR EXISTS "${TCL_DIR}/bin/tclsh84.exe")
  message(STATUS "Not Packaging TCL (cannot found: ${TCL_DIR}/bin/tclsh8.4 or ${TCL_DIR}/bin/tclsh84.exe)")
  set(TCL_DIR "")
endif(EXISTS "${TCL_DIR}/bin/tclsh8.4" OR EXISTS "${TCL_DIR}/bin/tclsh84.exe")

if(TCL_DIR)
  #SET(CPACK_INSTALLED_DIRECTORIES ${CPACK_INSTALLED_DIRECTORIES}
  #  "${TCL_DIR}" "/${Slicer3_INSTALL_LIB_DIR}/tcl")
  install(CODE "EXECUTE_PROCESS\(COMMAND \"${CMAKE_COMMAND}\" -E copy_directory \"${TCL_DIR}\" \"\${CMAKE_INSTALL_PREFIX}/lib/Slicer3/tcl\"\)")
endif(TCL_DIR)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in"
  "${Slicer3_BINARY_DIR}/PackageLibraries/CMakeLists.txt"
  @ONLY)


# If the cmake version includes cpack, use it
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

set(CPACK_PACKAGE_EXECUTABLES "..\\\\Slicer3" "Slicer")
include(CPack)

