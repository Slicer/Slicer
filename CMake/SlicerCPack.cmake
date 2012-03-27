if(Slicer_USE_PYTHONQT)
  # Python install rules are common to both 'bundled' and 'regular' package
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallPython.cmake)
endif()
if(Slicer_USE_PYTHONQT_WITH_TCL)
  # Tcl install rules are common to both 'bundled' and 'regular' package
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallTcl.cmake)
endif()

set(SlicerBlockInstallQtPlugins_subdirectories imageformats sqldrivers)
include(${Slicer_CMAKE_DIR}/SlicerBlockInstallQtPlugins.cmake)

include(${Slicer_CMAKE_DIR}/SlicerBlockInstallDCMTKApps.cmake)

set(CPACK_INSTALL_CMAKE_PROJECTS)

# Install CTK Apps and Plugins (PythonQt modules, QtDesigner plugins ...)
if(NOT "${CTK_DIR}" STREQUAL "" AND EXISTS "${CTK_DIR}/CTK-build/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;RuntimeApplications;/")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;RuntimePlugins;/")
endif()

if(NOT APPLE)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallQt.cmake)
  if(Slicer_USE_PYTHONQT)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallPythonQt.cmake)
  endif()
  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallLibArchive.cmake)
  endif()
  include(InstallRequiredSystemLibraries)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallCMakeProjects.cmake)
else()

  if(Slicer_USE_PYTHONQT)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallExternalPythonModules.cmake)
  endif()

  # Generate qt.conf
  file(WRITE ${Slicer_BINARY_DIR}/CMake/qt.conf-to-install
"[Paths]
Plugins = ${Slicer_QtPlugins_DIR}
")
  # .. and install
  install(FILES ${Slicer_BINARY_DIR}/CMake/qt.conf-to-install
          DESTINATION ${Slicer_INSTALL_ROOT}Resources
          COMPONENT Runtime
          RENAME qt.conf)

  set(executable_path @executable_path)
  set(slicer_cpack_bundle_fixup_directory ${Slicer_BINARY_DIR}/CMake/SlicerCPackBundleFixup)
  configure_file(
    "${Slicer_SOURCE_DIR}/CMake/SlicerCPackBundleFixup.cmake.in"
    "${slicer_cpack_bundle_fixup_directory}/SlicerCPackBundleFixup.cmake"
    @ONLY)
  # HACK - For a given directory, "install(SCRIPT ...)" rule will be evaluated first,
  #        let's make sure the following install rule is evaluated within its own directory.
  #        Otherwise, the associated script will be executed before any other relevant install rules.
  file(WRITE ${slicer_cpack_bundle_fixup_directory}/CMakeLists.txt
    "install(SCRIPT \"${slicer_cpack_bundle_fixup_directory}/SlicerCPackBundleFixup.cmake\")")
  add_subdirectory(${slicer_cpack_bundle_fixup_directory} ${slicer_cpack_bundle_fixup_directory}-binary)
endif()

# Install Slicer
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Slicer_BINARY_DIR};Slicer;ALL;/")

# -------------------------------------------------------------------------
# Package properties
# -------------------------------------------------------------------------
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Medical Visualization and Processing Environment for Research")

set(CPACK_MONOLITHIC_INSTALL ON)

set(CPACK_PACKAGE_VENDOR "NA-MIC")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${Slicer_SOURCE_DIR}/README.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${Slicer_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Slicer_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Slicer_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${Slicer_VERSION_PATCH}")
SET(CPACK_PACKAGE_VERSION "${Slicer_VERSION_FULL}")
set(CPACK_SYSTEM_NAME "${Slicer_PLATFORM}-${Slicer_ARCHITECTURE}")

if(APPLE)
  set(CPACK_PACKAGE_ICON "${Slicer_SOURCE_DIR}/Applications/SlicerQT/Resources/Slicer.icns")
endif()

# Slicer does *NOT* require setting the windows path
set(CPACK_NSIS_MODIFY_PATH OFF)

set(APPLICATION_NAME "Slicer")
set(EXECUTABLE_NAME "Slicer")
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
# Disable source generator enabled by default
# -------------------------------------------------------------------------
set(CPACK_SOURCE_TBZ2 OFF CACHE BOOL "Enable to build TBZ2 source packages" FORCE)
set(CPACK_SOURCE_TGZ  OFF CACHE BOOL "Enable to build TGZ source packages" FORCE)
set(CPACK_SOURCE_TZ   OFF CACHE BOOL "Enable to build TZ source packages" FORCE)

# -------------------------------------------------------------------------
# Enable generator
# -------------------------------------------------------------------------
if(UNIX)
  set(CPACK_GENERATOR "TGZ")
  if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
  endif()
elseif(WIN32)
  set(CPACK_GENERATOR "NSIS")
endif()

include(CPack)

