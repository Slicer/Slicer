
if(NOT APPLE)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallTcl.cmake)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallPython.cmake)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallQt.cmake)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallPythonQt.cmake)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallLibArchive.cmake)
  include(InstallRequiredSystemLibraries)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallCMakeProjects.cmake)
endif(NOT APPLE)

# -------------------------------------------------------------------------
# Package properties
# -------------------------------------------------------------------------
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Medical Visualization and Processing Environment for Research")

SET(CPACK_MONOLITHIC_INSTALL ON)

set(CPACK_PACKAGE_VENDOR "NA-MIC")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${Slicer_SOURCE_DIR}/README.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${Slicer_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Slicer_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Slicer_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${Slicer_VERSION_PATCH}")

if(APPLE)
  set(CPACK_PACKAGE_ICON "${Slicer_SOURCE_DIR}/Applications/SlicerQT/Resources/Slicer.icns")
endif()

# Slicer does *NOT* require setting the windows path
set(CPACK_NSIS_MODIFY_PATH OFF)

set(APPLICATION_NAME "Slicer")
set(EXECUTABLE_NAME "Slicer4")
set(CPACK_PACKAGE_EXECUTABLES "..\\\\${EXECUTABLE_NAME}" "${APPLICATION_NAME}")

# -------------------------------------------------------------------------
# File extensions
# -------------------------------------------------------------------------
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
    endforeach()
  endif()
endif()

# -------------------------------------------------------------------------
# Disable generator enabled by default
# -------------------------------------------------------------------------
set(CPACK_BINARY_TZ   OFF CACHE BOOL "Enable to build TZ packages" FORCE)
set(CPACK_BINARY_STGZ OFF CACHE BOOL "Enable to build STGZ packages" FORCE)
set(CPACK_SOURCE_TBZ2 OFF CACHE BOOL "Enable to build TBZ2 source packages" FORCE)
set(CPACK_SOURCE_TGZ  OFF CACHE BOOL "Enable to build TGZ source packages" FORCE)
set(CPACK_SOURCE_TZ   OFF CACHE BOOL "Enable to build TZ source packages" FORCE)

if(APPLE)
  set(CPACK_BINARY_PACKAGEMAKER OFF CACHE BOOL "Enable to build PackageMaker packages" FORCE)
endif()

# -------------------------------------------------------------------------
# Enable generator disabled by default
# -------------------------------------------------------------------------
if(APPLE)
  set(CPACK_BINARY_DRAGNDROP ON CACHE BOOL "Enable to build OSX Drag And Drop packages" FORCE)
endif()

include(CPack)

