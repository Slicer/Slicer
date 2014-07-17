
if(Slicer_USE_PYTHONQT AND NOT Slicer_USE_SYSTEM_python)
  # Python install rules are common to both 'bundled' and 'regular' package
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallPython.cmake)
endif()
if(Slicer_USE_PYTHONQT_WITH_TCL AND NOT Slicer_USE_SYSTEM_tcl)
  # Tcl install rules are common to both 'bundled' and 'regular' package
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallTcl.cmake)
endif()

if(NOT Slicer_USE_SYSTEM_QT)
  set(SlicerBlockInstallQtPlugins_subdirectories imageformats sqldrivers designer:qwebview)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallQtPlugins.cmake)
endif()

if(Slicer_BUILD_DICOM_SUPPORT AND NOT Slicer_USE_SYSTEM_DCMTK)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallDCMTKApps.cmake)
endif()

set(CPACK_INSTALL_CMAKE_PROJECTS)

# Install CTK Apps and Plugins (PythonQt modules, QtDesigner plugins ...)
if(NOT "${CTK_DIR}" STREQUAL "" AND EXISTS "${CTK_DIR}/CTK-build/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;RuntimeApplications;/")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;RuntimePlugins;/")
endif()

if(NOT APPLE)
  if(NOT Slicer_USE_SYSTEM_QT)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallQt.cmake)
  endif()
  if(Slicer_BUILD_DICOM_SUPPORT AND NOT Slicer_USE_SYSTEM_DCMTK)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallDCMTKLibs.cmake)
  endif()
  if(Slicer_USE_PYTHONQT AND NOT Slicer_USE_SYSTEM_CTK)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallPythonQt.cmake)
  endif()
  if(Slicer_USE_PYTHONQT_WITH_OPENSSL AND NOT Slicer_USE_SYSTEM_OpenSSL)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallOpenSSL.cmake)
  endif()
  if(Slicer_USE_QtTesting AND NOT Slicer_USE_SYSTEM_CTK)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallQtTesting.cmake)
  endif()
  if(NOT Slicer_USE_SYSTEM_LibArchive)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallLibArchive.cmake)
  endif()
  include(InstallRequiredSystemLibraries)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallCMakeProjects.cmake)

  macro(_remove_installed_dir dir_to_remove)
    set(_code "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E remove_directory")
    set(_code "${_code} \"${dollar}{CMAKE_INSTALL_PREFIX}/${Slicer_INSTALL_ROOT}${dir_to_remove}\")")
    install(CODE "${_code}" COMPONENT Runtime)
  endmacro()

  if(Slicer_INSTALL_NO_DEVELOPMENT)
    # Remove development files installed by teem. Ideally, teem project itself should be updated.
    # See http://na-mic.org/Mantis/view.php?id=3455
    set(dollar "$")
    _remove_installed_dir("include/teem")
    foreach(file
      lib/Teem-1.10.0/TeemBuildSettings.cmake
      lib/Teem-1.10.0/TeemConfig.cmake
      lib/Teem-1.10.0/TeemLibraryDepends.cmake
      lib/Teem-1.10.0/TeemUse.cmake
      )
      install(
        CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E remove \"${dollar}{CMAKE_INSTALL_PREFIX}/${Slicer_INSTALL_ROOT}${file}\")"
        COMPONENT Runtime
        )
    endforeach()
  endif()
else()

  set(CMAKE_INSTALL_NAME_TOOL "" CACHE FILEPATH "" FORCE)

  if(Slicer_USE_PYTHONQT)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallExternalPythonModules.cmake)
  endif()

  # Calling find_package will ensure the *_LIBRARY_DIRS expected by the fixup script are set.
  if(Slicer_USE_OpenIGTLink)
    find_package(OpenIGTLink REQUIRED)
  endif()
  if(Slicer_BUILD_CLI_SUPPORT)
    find_package(SlicerExecutionModel REQUIRED)
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

  set(fixup_path @rpath)
  set(slicer_cpack_bundle_fixup_directory ${Slicer_BINARY_DIR}/CMake/SlicerCPackBundleFixup)
  configure_file(
    "${Slicer_SOURCE_DIR}/CMake/SlicerCPackBundleFixup.cmake.in"
    "${slicer_cpack_bundle_fixup_directory}/SlicerCPackBundleFixup.cmake"
    @ONLY)
  # HACK - For a given directory, "install(SCRIPT ...)" rule will be evaluated first,
  #        let's make sure the following install rule is evaluated within its own directory.
  #        Otherwise, the associated script will be executed before any other relevant install rules.
  file(WRITE ${slicer_cpack_bundle_fixup_directory}/CMakeLists.txt
    "install(SCRIPT \"${slicer_cpack_bundle_fixup_directory}/SlicerCPackBundleFixup.cmake\" COMPONENT Runtime)")
  add_subdirectory(${slicer_cpack_bundle_fixup_directory} ${slicer_cpack_bundle_fixup_directory}-binary)

endif()

# Install additional projects if any, but also do a find_package to load CPACK
# variables of the Slicer_MAIN_PROJECT if different from SlicerApp
set(additional_projects ${Slicer_ADDITIONAL_DEPENDENCIES} ${Slicer_ADDITIONAL_PROJECTS})
foreach(additional_project ${additional_projects})
  if(NOT Slicer_USE_SYSTEM_${additional_project})
    find_package(${additional_project})
    if(${additional_project}_FOUND)
      if(${additional_project}_USE_FILE)
        include(${${additional_project}_USE_FILE})
      endif()
      if(NOT APPLE)
        if(DEFINED ${additional_project}_CPACK_INSTALL_CMAKE_PROJECTS)
          set(CPACK_INSTALL_CMAKE_PROJECTS
            "${CPACK_INSTALL_CMAKE_PROJECTS};${${additional_project}_CPACK_INSTALL_CMAKE_PROJECTS}")
        endif()
      endif()
    endif()
  endif()
endforeach()

# Install Slicer
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Slicer_BINARY_DIR};Slicer;RuntimeLibraries;/")
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Slicer_BINARY_DIR};Slicer;RuntimePlugins;/")
if(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Slicer_BINARY_DIR};Slicer;Development;/")
endif()
# Installation of 'Runtime' should be last to ensure the 'SlicerCPackBundleFixup.cmake' is executed last.
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Slicer_BINARY_DIR};Slicer;Runtime;/")

# -------------------------------------------------------------------------
# Package properties
# -------------------------------------------------------------------------
set(CPACK_MONOLITHIC_INSTALL ON)

set(Slicer_CPACK_PACKAGE_NAME ${SlicerApp_APPLICATION_NAME})
set(Slicer_CPACK_PACKAGE_VENDOR "NA-MIC")
set(Slicer_CPACK_PACKAGE_DESCRIPTION_FILE "${Slicer_SOURCE_DIR}/README.txt")
set(Slicer_CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "Medical Visualization and Processing Environment for Research")
set(Slicer_CPACK_RESOURCE_FILE_LICENSE "${Slicer_SOURCE_DIR}/License.txt")
set(Slicer_CPACK_PACKAGE_VERSION_MAJOR "${Slicer_VERSION_MAJOR}")
set(Slicer_CPACK_PACKAGE_VERSION_MINOR "${Slicer_VERSION_MINOR}")
set(Slicer_CPACK_PACKAGE_VERSION_PATCH "${Slicer_VERSION_PATCH}")
set(Slicer_CPACK_PACKAGE_VERSION "${Slicer_VERSION_FULL}")
set(Slicer_CPACK_PACKAGE_ICON "${Slicer_SOURCE_DIR}/Resources/Slicer.icns")

set(project ${${Slicer_MAIN_PROJECT}_APPLICATION_NAME})

set(CPACK_PACKAGE_NAME ${${project}_CPACK_PACKAGE_NAME})
set(CPACK_PACKAGE_VENDOR ${${project}_CPACK_PACKAGE_VENDOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${${project}_CPACK_PACKAGE_DESCRIPTION_SUMMARY})
set(CPACK_PACKAGE_DESCRIPTION_FILE ${${project}_CPACK_PACKAGE_DESCRIPTION_FILE})
set(CPACK_RESOURCE_FILE_LICENSE ${${project}_CPACK_RESOURCE_FILE_LICENSE})
set(CPACK_PACKAGE_VERSION_MAJOR ${${project}_CPACK_PACKAGE_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${${project}_CPACK_PACKAGE_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${${project}_CPACK_PACKAGE_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${${project}_CPACK_PACKAGE_VERSION})
set(CPACK_SYSTEM_NAME "${Slicer_OS}-${Slicer_ARCHITECTURE}")

if(APPLE)
  set(CPACK_PACKAGE_ICON ${${project}_CPACK_PACKAGE_ICON})
endif()

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")

# Installers for 32- vs. 64-bit CMake:
#  - Root install directory (displayed to end user at installer-run time)
#  - "NSIS package/display name" (text used in the installer GUI)
#  - Registry key used to store info about the installation
if(CMAKE_CL_64)
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION} (Win64)")
else()
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
  set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} (Win32)")
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
endif()

# Slicer does *NOT* require setting the windows path
set(CPACK_NSIS_MODIFY_PATH OFF)

set(APPLICATION_NAME "${Slicer_MAIN_PROJECT_APPLICATION_NAME}")
set(EXECUTABLE_NAME "${Slicer_MAIN_PROJECT_APPLICATION_NAME}")
set(CPACK_PACKAGE_EXECUTABLES "..\\\\${EXECUTABLE_NAME}" "${APPLICATION_NAME}")

# -------------------------------------------------------------------------
# File extensions
# -------------------------------------------------------------------------
set(FILE_EXTENSIONS .mrml .xcat .mrb)

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

