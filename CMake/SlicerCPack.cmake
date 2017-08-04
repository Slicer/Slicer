
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

include(${Slicer_CMAKE_DIR}/SlicerBlockInstallExtensionPackages.cmake)

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
  # XXX Note that installation of OpenMP libraries is available only
  #     when using msvc compiler.
  if(NOT DEFINED CMAKE_INSTALL_OPENMP_LIBRARIES)
    set(CMAKE_INSTALL_OPENMP_LIBRARIES ON)
  endif()
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT "RuntimeLibraries")
  include(InstallRequiredSystemLibraries)

  include(${Slicer_CMAKE_DIR}/SlicerBlockInstallCMakeProjects.cmake)

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
  set(VTK_LIBRARY_DIRS "${VTK_DIR}/lib")

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
# Common package properties
# -------------------------------------------------------------------------
set(CPACK_MONOLITHIC_INSTALL ON)

set(Slicer_CPACK_PACKAGE_NAME ${SlicerApp_APPLICATION_NAME})
set(Slicer_CPACK_PACKAGE_VENDOR ${Slicer_ORGANIZATION_NAME})
set(Slicer_CPACK_RESOURCE_FILE_LICENSE "${Slicer_SOURCE_DIR}/License.txt")
set(Slicer_CPACK_PACKAGE_VERSION_MAJOR "${Slicer_VERSION_MAJOR}")
set(Slicer_CPACK_PACKAGE_VERSION_MINOR "${Slicer_VERSION_MINOR}")
set(Slicer_CPACK_PACKAGE_VERSION_PATCH "${Slicer_VERSION_PATCH}")
set(Slicer_CPACK_PACKAGE_VERSION "${Slicer_VERSION_FULL}")
set(Slicer_CPACK_PACKAGE_INSTALL_DIRECTORY "${Slicer_CPACK_PACKAGE_NAME} ${Slicer_CPACK_PACKAGE_VERSION}")

set(project ${${Slicer_MAIN_PROJECT}_APPLICATION_NAME})

# Get main application properties
get_property(${project}_CPACK_PACKAGE_DESCRIPTION_FILE GLOBAL PROPERTY ${project}_DESCRIPTION_FILE)
get_property(${project}_CPACK_PACKAGE_DESCRIPTION_SUMMARY GLOBAL PROPERTY ${project}_DESCRIPTION_SUMMARY)
get_property(${project}_CPACK_PACKAGE_ICON GLOBAL PROPERTY ${project}_APPLE_ICON_FILE)

function(slicer_verbose_set varname)
  message(STATUS "Setting ${varname} to '${ARGN}'")
  set(${varname} "${ARGN}" PARENT_SCOPE)
endfunction()

macro(slicer_cpack_set varname)
  if(DEFINED ${project}_${varname})
    slicer_verbose_set(${varname} ${${project}_${varname}})
  elseif(DEFINED Slicer_${varname})
    slicer_verbose_set(${varname} ${Slicer_${varname}})
  else()
    if(NOT "Slicer" STREQUAL "${project}")
      set(_error_msg "Neither Slicer_${varname} or ${project}_${varname} are defined.")
    else()
      set(_error_msg "${project}_${varname} is not defined")
    endif()
    message(FATAL_ERROR "Failed to set variable ${varname}. ${_error_msg}")
  endif()
endmacro()

slicer_cpack_set("CPACK_PACKAGE_NAME")
slicer_cpack_set("CPACK_PACKAGE_VENDOR")
slicer_cpack_set("CPACK_PACKAGE_DESCRIPTION_SUMMARY")
slicer_cpack_set("CPACK_PACKAGE_DESCRIPTION_FILE")
slicer_cpack_set("CPACK_RESOURCE_FILE_LICENSE")
slicer_cpack_set("CPACK_PACKAGE_VERSION_MAJOR")
slicer_cpack_set("CPACK_PACKAGE_VERSION_MINOR")
slicer_cpack_set("CPACK_PACKAGE_VERSION_PATCH")
slicer_cpack_set("CPACK_PACKAGE_VERSION")
set(CPACK_SYSTEM_NAME "${Slicer_OS}-${Slicer_ARCHITECTURE}")
slicer_cpack_set("CPACK_PACKAGE_INSTALL_DIRECTORY")

if(APPLE)
  slicer_cpack_set("CPACK_PACKAGE_ICON")
endif()

# -------------------------------------------------------------------------
# NSIS package properties
# -------------------------------------------------------------------------
if(CPACK_GENERATOR STREQUAL "NSIS")

  set(Slicer_CPACK_NSIS_INSTALL_SUBDIRECTORY "")
  slicer_cpack_set("CPACK_NSIS_INSTALL_SUBDIRECTORY")

  # Installers for 32- vs. 64-bit CMake:
  #  - Root install directory (displayed to end user at installer-run time)
  #  - "NSIS package/display name" (text used in the installer GUI)
  #  - Registry key used to store info about the installation
  if(CMAKE_CL_64)
    set(_nsis_install_root "$PROGRAMFILES64")
    slicer_verbose_set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    slicer_verbose_set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_INSTALL_DIRECTORY} (Win64)")
  else()
    set(_nsis_install_root "$PROGRAMFILES")
    slicer_verbose_set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} (Win32)")
    slicer_verbose_set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
  endif()

  if(NOT CPACK_NSIS_INSTALL_SUBDIRECTORY STREQUAL "")
    set(_nsis_install_root "${_nsis_install_root}\\\\${CPACK_NSIS_INSTALL_SUBDIRECTORY}")
  endif()
  slicer_verbose_set(CPACK_NSIS_INSTALL_ROOT ${_nsis_install_root})

  # Slicer does *NOT* require setting the windows path
  set(CPACK_NSIS_MODIFY_PATH OFF)

  set(APPLICATION_NAME "${Slicer_MAIN_PROJECT_APPLICATION_NAME}")
  set(EXECUTABLE_NAME "${Slicer_MAIN_PROJECT_APPLICATION_NAME}")
  slicer_verbose_set(CPACK_PACKAGE_EXECUTABLES "..\\\\${EXECUTABLE_NAME}" "${APPLICATION_NAME}")

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

include(CPack)

