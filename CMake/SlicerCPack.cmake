
# -------------------------------------------------------------------------
# Disable source generator enabled by default
# -------------------------------------------------------------------------
set(CPACK_SOURCE_TBZ2 OFF CACHE BOOL "Enable to build TBZ2 source packages" FORCE)
set(CPACK_SOURCE_TGZ  OFF CACHE BOOL "Enable to build TGZ source packages" FORCE)
set(CPACK_SOURCE_TZ   OFF CACHE BOOL "Enable to build TZ source packages" FORCE)

# -------------------------------------------------------------------------
# Select generator
# -------------------------------------------------------------------------
if(UNIX)
  set(CPACK_GENERATOR "TGZ")
  if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
  endif()
elseif(WIN32)
  set(CPACK_GENERATOR "NSIS")
endif()

# -------------------------------------------------------------------------
# Install standalone executable and plugins
# -------------------------------------------------------------------------

if(Slicer_USE_PYTHONQT AND NOT Slicer_USE_SYSTEM_python)
  # Python install rules are common to both 'bundled' and 'regular' package
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallPython.cmake)
endif()

if(NOT Slicer_USE_SYSTEM_QT)
  set(SlicerBlockInstallQtPlugins_subdirectories
    audio
    imageformats
    printsupport
    sqldrivers
    )
    if(Slicer_BUILD_WEBENGINE_SUPPORT)
      list(APPEND SlicerBlockInstallQtPlugins_subdirectories
        designer:webengineview
        )
    endif()
    if(APPLE)
      list(APPEND SlicerBlockInstallQtPlugins_subdirectories
        platforms:cocoa
        )
    elseif(UNIX)
      list(APPEND SlicerBlockInstallQtPlugins_subdirectories
        platforms:xcb
        xcbglintegrations:xcb-glx-integration
        )
    elseif(WIN32)
      list(APPEND SlicerBlockInstallQtPlugins_subdirectories
        platforms:windows
        )
    endif()
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallQtPlugins.cmake)
endif()

if(Slicer_BUILD_DICOM_SUPPORT AND NOT Slicer_USE_SYSTEM_DCMTK)
  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallDCMTKApps.cmake)
endif()

# Install Qt designer launcher
if(Slicer_BUILD_QT_DESIGNER_PLUGINS)
  set(executablename "SlicerDesigner")
  set(build_designer_executable "${QT_BINARY_DIR}/designer${CMAKE_EXECUTABLE_SUFFIX}")
  if(APPLE)
    set(build_designer_executable "${QT_BINARY_DIR}/Designer.app/Contents/MacOS/designer")
  endif()
  set(installed_designer_executable "designer-real${CMAKE_EXECUTABLE_SUFFIX}")
  set(installed_designer_subdir ".")
  if(APPLE)
    set(installed_designer_executable "Designer")
    set(installed_designer_subdir "Designer.app/Contents/MacOS")
  endif()
  # Ensure directory exists at configuration time
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR})
  # Configure designer launcher
  find_package(CTKAppLauncher REQUIRED)
  ctkAppLauncherConfigureForExecutable(
    APPLICATION_NAME ${executablename}
    SPLASHSCREEN_DISABLED
    # Additional settings exclude groups
    ADDITIONAL_SETTINGS_EXCLUDE_GROUPS "General,Application,ExtraApplicationToLaunch"
    # Launcher settings specific to build tree
    APPLICATION_EXECUTABLE ${build_designer_executable}
    DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR}
    ADDITIONAL_SETTINGS_FILEPATH_BUILD "${Slicer_BINARY_DIR}/${Slicer_MAIN_PROJECT_APPLICATION_NAME}LauncherSettings.ini"
    # Launcher settings specific to install tree
    APPLICATION_INSTALL_EXECUTABLE_NAME "${installed_designer_executable}"
    APPLICATION_INSTALL_SUBDIR "${installed_designer_subdir}"
    ADDITIONAL_SETTINGS_FILEPATH_INSTALLED "<APPLAUNCHER_SETTINGS_DIR>/${Slicer_MAIN_PROJECT_APPLICATION_NAME}LauncherSettings.ini"
    )
  # Install designer launcher settings
  install(
    FILES ${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR}/${executablename}LauncherSettingsToInstall.ini
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    RENAME ${executablename}LauncherSettings.ini
    COMPONENT Runtime
    )
  # Install designer launcher
  set(_launcher CTKAppLauncher)
  if(Slicer_BUILD_WIN32_CONSOLE)
    set(_launcher CTKAppLauncherW)
  endif()
  install(
    PROGRAMS ${CTKAppLauncher_DIR}/bin/${_launcher}${CMAKE_EXECUTABLE_SUFFIX}
    DESTINATION ${Slicer_INSTALL_BIN_DIR}
    RENAME ${executablename}${CMAKE_EXECUTABLE_SUFFIX}
    COMPONENT Runtime
    )
endif()

# -------------------------------------------------------------------------
# Update CPACK_INSTALL_CMAKE_PROJECTS
# -------------------------------------------------------------------------
set(CPACK_INSTALL_CMAKE_PROJECTS)

# Ensure external project associated with bundled extensions are packaged
foreach(extension_name ${Slicer_BUNDLED_EXTENSION_NAMES})
  if(DEFINED "${extension_name}_CPACK_INSTALL_CMAKE_PROJECTS")
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${${extension_name}_CPACK_INSTALL_CMAKE_PROJECTS}")
  endif()
endforeach()

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
  if(Slicer_USE_TBB AND NOT Slicer_USE_SYSTEM_TBB)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallTBB.cmake)
  endif()
  if(Slicer_USE_QtTesting AND NOT Slicer_USE_SYSTEM_CTK)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallQtTesting.cmake)
  endif()
  if(NOT Slicer_USE_SYSTEM_LibArchive)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallLibArchive.cmake)
  endif()
  # XXX Note that installation of OpenMP libraries is available only
  #     when using msvc compiler.
  if(NOT DEFINED CMAKE_INSTALL_OPENMP_LIBRARIES)
    set(CMAKE_INSTALL_OPENMP_LIBRARIES ON)
  endif()
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT "RuntimeLibraries")
  include(InstallRequiredSystemLibraries)

  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallCMakeProjects.cmake)

else()

  #------------------------------------------------------------------------------
  # macOS specific configuration used by the "fix-up" script
  #------------------------------------------------------------------------------
  set(CMAKE_INSTALL_NAME_TOOL "" CACHE FILEPATH "" FORCE)

  if(Slicer_USE_PYTHONQT)
    include(${Slicer_CMAKE_DIR}/SlicerBlockInstallExternalPythonModules.cmake)
  endif()

  # Calling find_package will ensure the *_LIBRARY_DIRS expected by the fixup script are set.
  if(Slicer_BUILD_CLI_SUPPORT)
    find_package(SlicerExecutionModel REQUIRED)
  endif()
  set(VTK_LIBRARY_DIRS "${VTK_DIR}/lib")

  # Get Qt root directory
  get_property(_filepath TARGET "Qt5::Core" PROPERTY LOCATION_RELEASE)
  get_filename_component(_dir ${_filepath} PATH)
  set(qt_root_dir "${_dir}/..")

  #------------------------------------------------------------------------------
  # <ExtensionName>_FIXUP_BUNDLE_LIBRARY_DIRECTORIES
  #------------------------------------------------------------------------------

  #
  # Setting this variable in the CMakeLists.txt of an extension allows to update
  # the list of directories used by the "fix-up" script to look up libraries
  # that should be copied into the Slicer package when the extension is bundled.
  #
  # To ensure the extension can be bundled, the variable should be set as a CACHE
  # variable.
  #

  set(EXTENSION_BUNDLE_FIXUP_LIBRARY_DIRECTORIES)
  foreach(project ${Slicer_BUNDLED_EXTENSION_NAMES})
    if(DEFINED ${project}_FIXUP_BUNDLE_LIBRARY_DIRECTORIES)
      # Exclude system directories.
      foreach(lib_path IN LISTS ${project}_FIXUP_BUNDLE_LIBRARY_DIRECTORIES)
        if(lib_path MATCHES "^(/lib|/lib32|/libx32|/lib64|/usr/lib|/usr/lib32|/usr/libx32|/usr/lib64|/usr/X11R6|/usr/bin)"
            OR lib_path MATCHES "^(/System/Library|/usr/lib)")
          continue()
        endif()
        list(APPEND EXTENSION_BUNDLE_FIXUP_LIBRARY_DIRECTORIES ${lib_path})
      endforeach()
    endif()
  endforeach()

  #------------------------------------------------------------------------------
  # Configure "fix-up" script
  #------------------------------------------------------------------------------
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

include(${Slicer_CMAKE_DIR}/SlicerBlockInstallExtensionPackages.cmake)

# -------------------------------------------------------------------------
# Update CPACK_INSTALL_CMAKE_PROJECTS
# -------------------------------------------------------------------------

# Install additional projects if any, but also do a find_package to load CPACK
# variables of the Slicer_MAIN_PROJECT if different from SlicerApp
set(additional_projects ${Slicer_ADDITIONAL_DEPENDENCIES} ${Slicer_ADDITIONAL_PROJECTS})
foreach(additional_project ${additional_projects})
  if(NOT Slicer_USE_SYSTEM_${additional_project})
    find_package(${additional_project} QUIET)
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
# Define helper macros and functions
# -------------------------------------------------------------------------
function(slicer_verbose_set varname)
  message(STATUS "Setting ${varname} to '${ARGN}'")
  set(${varname} "${ARGN}" PARENT_SCOPE)
endfunction()

# Convenience variable used below: This is the name of the application (e.g Slicer)
# whereas Slicer_MAIN_PROJECT is the application project name (e.g SlicerApp, AwesomeApp, ...)
set(app_name ${${Slicer_MAIN_PROJECT}_APPLICATION_NAME})

macro(slicer_cpack_set varname)
  if(DEFINED ${app_name}_${varname})
    slicer_verbose_set(${varname} ${${app_name}_${varname}})
  elseif(DEFINED Slicer_${varname})
    slicer_verbose_set(${varname} ${Slicer_${varname}})
  else()
    if(NOT "Slicer" STREQUAL "${app_name}")
      set(_error_msg "Neither Slicer_${varname} or ${app_name}_${varname} are defined.")
    else()
      set(_error_msg "${app_name}_${varname} is not defined")
    endif()
    message(FATAL_ERROR "Failed to set variable ${varname}. ${_error_msg}")
  endif()
endmacro()

# -------------------------------------------------------------------------
# Common package properties
# -------------------------------------------------------------------------
set(CPACK_MONOLITHIC_INSTALL ON)
if(UNIX AND NOT APPLE AND "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  # Reduce package size stripping symbols from the regular symbol table
  # for ELF libraries and executables.
  # See also use of slicerInstallLibrary() and slicerStripInstalledLibrary() functions
  set(CPACK_STRIP_FILES 1)
endif()

set(${app_name}_CPACK_PACKAGE_NAME ${app_name})
slicer_cpack_set("CPACK_PACKAGE_NAME")

set(Slicer_CPACK_PACKAGE_VENDOR ${Slicer_ORGANIZATION_NAME})
slicer_cpack_set("CPACK_PACKAGE_VENDOR")

set(Slicer_CPACK_PACKAGE_VERSION_MAJOR "${Slicer_VERSION_MAJOR}")
set(${app_name}_CPACK_PACKAGE_VERSION_MAJOR ${Slicer_MAIN_PROJECT_VERSION_MAJOR})
slicer_cpack_set("CPACK_PACKAGE_VERSION_MAJOR")

set(Slicer_CPACK_PACKAGE_VERSION_MINOR "${Slicer_VERSION_MINOR}")
set(${app_name}_CPACK_PACKAGE_VERSION_MINOR ${Slicer_MAIN_PROJECT_VERSION_MINOR})
slicer_cpack_set("CPACK_PACKAGE_VERSION_MINOR")

set(Slicer_CPACK_PACKAGE_VERSION_PATCH "${Slicer_VERSION_PATCH}")
set(${app_name}_CPACK_PACKAGE_VERSION_PATCH ${Slicer_MAIN_PROJECT_VERSION_PATCH})
slicer_cpack_set("CPACK_PACKAGE_VERSION_PATCH")

set(Slicer_CPACK_PACKAGE_VERSION "${Slicer_VERSION_FULL}")
set(${app_name}_CPACK_PACKAGE_VERSION ${Slicer_MAIN_PROJECT_VERSION_FULL})
slicer_cpack_set("CPACK_PACKAGE_VERSION")

set(CPACK_SYSTEM_NAME "${Slicer_OS}-${Slicer_ARCHITECTURE}")

set(Slicer_CPACK_PACKAGE_INSTALL_DIRECTORY "${${app_name}_CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
slicer_cpack_set("CPACK_PACKAGE_INSTALL_DIRECTORY")

get_property(${app_name}_CPACK_PACKAGE_DESCRIPTION_FILE GLOBAL PROPERTY ${app_name}_DESCRIPTION_FILE)
slicer_cpack_set("CPACK_PACKAGE_DESCRIPTION_FILE")

get_property(${app_name}_CPACK_RESOURCE_FILE_LICENSE GLOBAL PROPERTY ${app_name}_LICENSE_FILE)
slicer_cpack_set("CPACK_RESOURCE_FILE_LICENSE")

get_property(${app_name}_CPACK_PACKAGE_DESCRIPTION_SUMMARY GLOBAL PROPERTY ${app_name}_DESCRIPTION_SUMMARY)
slicer_cpack_set("CPACK_PACKAGE_DESCRIPTION_SUMMARY")

get_property(${app_name}_CPACK_PACKAGE_ICON GLOBAL PROPERTY ${app_name}_APPLE_ICON_FILE)
if(APPLE)
  slicer_cpack_set("CPACK_PACKAGE_ICON")
endif()

# -------------------------------------------------------------------------
# NSIS package properties
# -------------------------------------------------------------------------
if(CPACK_GENERATOR STREQUAL "NSIS")

  set(Slicer_CPACK_NSIS_INSTALL_SUBDIRECTORY "")
  slicer_cpack_set("CPACK_NSIS_INSTALL_SUBDIRECTORY")

  set(_nsis_install_root "${Slicer_CPACK_NSIS_INSTALL_ROOT}")

  # Use ManifestDPIAware to improve appearance of installer
  string(APPEND CPACK_NSIS_DEFINES "\n  ;Use ManifestDPIAware to improve appearance of installer")
  string(APPEND CPACK_NSIS_DEFINES "\n  ManifestDPIAware true\n")

  # Use ManifestLongPathAware to support packaging of application where an install prefix like the following
  # would lead to paths having their length beyond the 260 characters limit:
  # "C:/path/to/Slicer-build/_CPack_Packages/win-amd64/NSIS/<Slicer_MAIN_PROJECT>-X.Y.Z-YYYY-MM-DD-win-amd64"
  string(APPEND CPACK_NSIS_DEFINES "\n  ;Use ManifestLongPathAware to support longer install prefix")
  string(APPEND CPACK_NSIS_DEFINES "\n  ManifestLongPathAware true\n")

  if (NOT Slicer_CPACK_NSIS_INSTALL_REQUIRES_ADMIN_ACCOUNT)
    # Install as regular user (UAC dialog will not be shown).
    string(APPEND CPACK_NSIS_DEFINES "\n  ;Install as regular user (UAC dialog will not be shown).")
    string(APPEND CPACK_NSIS_DEFINES "\n  RequestExecutionLevel user")
  endif()

  # Installers for 32- vs. 64-bit CMake:
  #  - "NSIS package/display name" (text used in the installer GUI)
  #  - Registry key used to store info about the installation

  if(CMAKE_CL_64)
    slicer_verbose_set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    slicer_verbose_set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_INSTALL_DIRECTORY} (Win64)")
  else()
    slicer_verbose_set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} (Win32)")
    slicer_verbose_set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
  endif()

  if(NOT CPACK_NSIS_INSTALL_SUBDIRECTORY STREQUAL "")
    set(_nsis_install_root "${_nsis_install_root}/${CPACK_NSIS_INSTALL_SUBDIRECTORY}")
  endif()
  string(REPLACE "/" "\\\\" _nsis_install_root "${_nsis_install_root}")
  slicer_verbose_set(CPACK_NSIS_INSTALL_ROOT ${_nsis_install_root})

  # Slicer does *NOT* require setting the windows path
  set(CPACK_NSIS_MODIFY_PATH OFF)

  set(APPLICATION_NAME "${Slicer_MAIN_PROJECT_APPLICATION_NAME}")
  set(EXECUTABLE_NAME "${Slicer_MAIN_PROJECT_APPLICATION_NAME}")
  # Set application name used to create Start Menu shortcuts
  set(PACKAGE_APPLICATION_NAME "${APPLICATION_NAME} ${CPACK_PACKAGE_VERSION}")
  slicer_verbose_set(CPACK_PACKAGE_EXECUTABLES "..\\\\${EXECUTABLE_NAME}" "${PACKAGE_APPLICATION_NAME}")

  get_property(${app_name}_CPACK_NSIS_MUI_ICON GLOBAL PROPERTY ${app_name}_WIN_ICON_FILE)
  slicer_cpack_set("CPACK_NSIS_MUI_ICON")
  slicer_verbose_set(CPACK_NSIS_INSTALLED_ICON_NAME "${app_name}.exe")
  slicer_verbose_set(CPACK_NSIS_MUI_FINISHPAGE_RUN "../${APPLICATION_NAME}.exe")

  # -------------------------------------------------------------------------
  # File extensions
  # -------------------------------------------------------------------------
  set(FILE_EXTENSIONS .mrml .xcat .mrb)

  if(FILE_EXTENSIONS)

    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS)
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS)
    foreach(ext ${FILE_EXTENSIONS})
      string(LENGTH "${ext}" len)
      math(EXPR len_m1 "${len} - 1")
      string(SUBSTRING "${ext}" 1 ${len_m1} ext_wo_dot)
      set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
WriteRegStr SHCTX \\\"SOFTWARE\\\\Classes\\\\${APPLICATION_NAME}\\\" \\\"\\\" \\\"${APPLICATION_NAME} supported file\\\"
WriteRegStr SHCTX \\\"SOFTWARE\\\\Classes\\\\${APPLICATION_NAME}\\\" \\\"URL Protocol\\\" \\\"\\\"
WriteRegStr SHCTX \\\"SOFTWARE\\\\Classes\\\\${APPLICATION_NAME}\\\\shell\\\\open\\\\command\\\" \
\\\"\\\" \\\"$\\\\\\\"$INSTDIR\\\\${EXECUTABLE_NAME}.exe$\\\\\\\" $\\\\\\\"%1$\\\\\\\"\\\"
WriteRegStr SHCTX \\\"SOFTWARE\\\\Classes\\\\${ext}\\\" \\\"\\\" \\\"${APPLICATION_NAME}\\\"
WriteRegStr SHCTX \\\"SOFTWARE\\\\Classes\\\\${ext}\\\" \\\"Content Type\\\" \\\"application/x-${ext_wo_dot}\\\"
")
      set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
DeleteRegKey SHCTX \\\"SOFTWARE\\\\Classes\\\\${APPLICATION_NAME}\\\"
DeleteRegKey SHCTX \\\"SOFTWARE\\\\Classes\\\\${ext}\\\"
")
    endforeach()
  endif()

endif()

include(CPack)

