cmake_minimum_required(VERSION 3.13.4)

foreach(varname IN ITEMS
  CMAKE_EXECUTABLE_SUFFIX
  )
  if(NOT DEFINED ${varname})
    message(FATAL_ERROR "${varname} is not defined")
  endif()
endforeach()

foreach(varname IN ITEMS
  CTKAppLauncher_DIR
  python_DIR
  PYTHON_ENABLE_SSL
  PYTHON_REAL_EXECUTABLE
  PYTHON_SHARED_LIBRARY_DIR
  PYTHON_SITE_PACKAGES_SUBDIR
  PYTHON_STDLIB_SUBDIR
  Slicer_BIN_DIR
  Slicer_BINARY_DIR
  Slicer_LIB_DIR
  Slicer_SHARE_DIR
  Slicer_SOURCE_DIR
  )
  if("${${varname}}" STREQUAL "")
    message(FATAL_ERROR "${varname} is empty")
  endif()
endforeach()

find_package(CTKAppLauncher REQUIRED)

get_filename_component(python_bin_dir "${PYTHON_REAL_EXECUTABLE}" PATH)

#
# Settings specific to the build tree.
#

set(PYTHONHOME "${python_DIR}")

# PATHS
set(PYTHONLAUNCHER_PATHS_BUILD
  <APPLAUNCHER_DIR>
  )

# LIBRARY_PATHS
set(PYTHONLAUNCHER_LIBRARY_PATHS_BUILD
  ${PYTHON_SHARED_LIBRARY_DIR}
  )
if(PYTHON_ENABLE_SSL)
  list(APPEND PYTHONLAUNCHER_LIBRARY_PATHS_BUILD
    ${OPENSSL_EXPORT_LIBRARY_DIR}
    )
endif()

# ENVVARS
set(PYTHONLAUNCHER_ENVVARS_BUILD
  "PYTHONHOME=${PYTHONHOME}"
  "PYTHONNOUSERSITE=1"
  "PIP_REQUIRE_VIRTUALENV=0"
  )
if(PYTHON_ENABLE_SSL)

  set(_src ${Slicer_SOURCE_DIR}/Base/QTCore/Resources/Certs/Slicer.crt)
  set(_dest ${Slicer_BINARY_DIR}/Slicer-build/${Slicer_SHARE_DIR}/Slicer.crt)
  configure_file(${_src} ${_dest} COPYONLY)
  message(STATUS "Copying '${_src}' to '${_dest}'")

  list(APPEND PYTHONLAUNCHER_ENVVARS_BUILD
    "SSL_CERT_FILE=<APPLAUNCHER_DIR>/../../Slicer-build/${Slicer_SHARE_DIR}/Slicer.crt"
    )
endif()

# PATH ENVVARS
set(PYTHONLAUNCHER_ADDITIONAL_PATH_ENVVARS_BUILD
  "PYTHONPATH"
  )
set(PYTHONLAUNCHER_PYTHONPATH_BUILD
  "${PYTHONHOME}/${PYTHON_STDLIB_SUBDIR}"
  "${PYTHONHOME}/${PYTHON_STDLIB_SUBDIR}/lib-dynload"
  "${PYTHONHOME}/${PYTHON_SITE_PACKAGES_SUBDIR}"
  )

#
# Settings specific to the install tree.
#
set(PYTHONHOME "<APPLAUNCHER_DIR>/../lib/Python")

# Windows:
#   Python library    -> Slicer_BIN_DIR
#   OpenSSL libraries -> Slicer_BIN_DIR
# Unix:
#   Python library    -> lib/Python/lib
#   OpenSSL libraries -> Slicer_LIB_DIR

# PATHS
set(PYTHONLAUNCHER_PATHS_INSTALLED
  <APPLAUNCHER_DIR>
  )

# LIBRARY_PATHS
set(PYTHONLAUNCHER_LIBRARY_PATHS_INSTALLED
  <APPLAUNCHER_DIR>/../${Slicer_BIN_DIR}
  <APPLAUNCHER_DIR>/../${Slicer_LIB_DIR}
  <APPLAUNCHER_DIR>/../lib/Python/lib
  )

# ENVVARS
set(PYTHONLAUNCHER_ENVVARS_INSTALLED
  "PYTHONHOME=${PYTHONHOME}"
  "PYTHONNOUSERSITE=1"
  "PIP_REQUIRE_VIRTUALENV=0"
  )

if(PYTHON_ENABLE_SSL)
  list(APPEND PYTHONLAUNCHER_ENVVARS_INSTALLED
    "SSL_CERT_FILE=<APPLAUNCHER_DIR>/../${Slicer_SHARE_DIR}/Slicer.crt"
    )
endif()

# PATH ENVVARS
set(PYTHONLAUNCHER_ADDITIONAL_PATH_ENVVARS_INSTALLED
  "PYTHONPATH"
  )
set(PYTHONLAUNCHER_PYTHONPATH_INSTALLED
  "${PYTHONHOME}/lib/Python/${PYTHON_STDLIB_SUBDIR}"
  "${PYTHONHOME}/lib/Python/${PYTHON_STDLIB_SUBDIR}/lib-dynload"
  "${PYTHONHOME}/lib/Python/${PYTHON_SITE_PACKAGES_SUBDIR}"
  "<APPLAUNCHER_DIR>/../${Slicer_BIN_DIR}/Python"
  )

#
# Notes:
#
#  * Install rules for PythonSlicerLauncherSettingsToInstall.ini and PythonSlicer executable
#  are specified in SlicerBlockInstallPython.cmake
#

set(_python_launcher_config_params
  SPLASHSCREEN_DISABLED

  # Additional settings exclude groups
  ADDITIONAL_SETTINGS_EXCLUDE_GROUPS "General,Application,ExtraApplicationToLaunch"

  # Additional path envars prefix
  ADDITIONAL_PATH_ENVVARS_PREFIX PYTHONLAUNCHER_

  # Launcher settings specific to build tree
  APPLICATION_EXECUTABLE ${PYTHON_REAL_EXECUTABLE}
  DESTINATION_DIR ${python_bin_dir}
  PATHS_BUILD "${PYTHONLAUNCHER_PATHS_BUILD}"
  LIBRARY_PATHS_BUILD "${PYTHONLAUNCHER_LIBRARY_PATHS_BUILD}"
  ENVVARS_BUILD "${PYTHONLAUNCHER_ENVVARS_BUILD}"
  ADDITIONAL_PATH_ENVVARS_BUILD "${PYTHONLAUNCHER_ADDITIONAL_PATH_ENVVARS_BUILD}"
  ADDITIONAL_SETTINGS_FILEPATH_BUILD "${Slicer_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/SlicerLauncherSettings.ini"

  # Launcher settings specific to install tree
  APPLICATION_INSTALL_EXECUTABLE_NAME python-real${CMAKE_EXECUTABLE_SUFFIX}
  APPLICATION_INSTALL_SUBDIR "."
  PATHS_INSTALLED "${PYTHONLAUNCHER_PATHS_INSTALLED}"
  LIBRARY_PATHS_INSTALLED "${PYTHONLAUNCHER_LIBRARY_PATHS_INSTALLED}"
  ENVVARS_INSTALLED "${PYTHONLAUNCHER_ENVVARS_INSTALLED}"
  ADDITIONAL_PATH_ENVVARS_INSTALLED "${PYTHONLAUNCHER_ADDITIONAL_PATH_ENVVARS_INSTALLED}"
  ADDITIONAL_SETTINGS_FILEPATH_INSTALLED "<APPLAUNCHER_SETTINGS_DIR>/SlicerLauncherSettings.ini"
  )

# Custom Python executable name must start with Python for compatibility with
# development tools, such as PyCharm.
ctkAppLauncherConfigureForExecutable(
  APPLICATION_NAME PythonSlicer
  ${_python_launcher_config_params}
  )

# SlicerPython executable name is deprecated and will be removed in the future
# (name is not compatible with some development environments - see the comment above).
file(WRITE "${python_bin_dir}/SlicerPython-is-obsolete.py" [==[
exit("""SlicerPython executable is obsolete and will be removed. Use PythonSlicer executable instead.
For more details, see https://github.com/Slicer/Slicer/issues/4843
""")
]==])
ctkAppLauncherConfigureForExecutable(
  APPLICATION_NAME SlicerPython
  APPLICATION_DEFAULT_ARGUMENTS "${python_bin_dir}/SlicerPython-is-obsolete.py"
  ${_python_launcher_config_params}
  )
