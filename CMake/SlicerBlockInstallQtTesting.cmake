# -------------------------------------------------------------------------
# Find and install QtTesting
# -------------------------------------------------------------------------
#
# QtTesting is built and installed by the CTK external project. Slicer
# packages the resulting shared library from the CTK install tree.
#
# The library file name is queried from the imported QtTesting CMake
# target rather than hard-coded, so that future upstream changes to the
# library output name (for example the Qt5+Qt6 rename of `QtTesting` to
# `qttesting`) do not silently break Slicer packaging.
#
# NOTE: This logic assumes upstream installs a single, unversioned shared
# library file (no SONAME symlink chain such as libqttesting.so.1 ->
# libqttesting.so). If a future QtTesting revision introduces SONAME
# versioning, this block must also install the symlink chain — for
# example by globbing `libqttesting.so*` or by installing both
# TARGET_SONAME_FILE and TARGET_FILE names.

set(QtTesting_INSTALL_LIB_DIR "${Slicer_INSTALL_LIB_DIR}")

find_package(QtTesting CONFIG QUIET REQUIRED)

# Probe known QtTesting imported-target names (current and likely-future
# spellings) so a future namespaced rename upstream is caught loudly.
set(_qttesting_target "")
foreach(_candidate qttesting QtTesting QtTesting::qttesting CTK::QtTesting)
  if(TARGET ${_candidate})
    set(_qttesting_target ${_candidate})
    break()
  endif()
endforeach()
if(NOT _qttesting_target)
  message(FATAL_ERROR
    "SlicerBlockInstallQtTesting: no imported QtTesting target was "
    "defined by find_package(QtTesting). Tried: qttesting, QtTesting, "
    "QtTesting::qttesting, CTK::QtTesting. Upstream may have renamed "
    "the target; extend the probe list above.")
endif()

# Resolve the shared library file name (e.g. libqttesting.so,
# libqttesting.dylib, qttesting.dll) from the imported target.
set(_qttesting_imported_location "")
foreach(_cfg
    "${CMAKE_BUILD_TYPE}"
    ${CMAKE_CONFIGURATION_TYPES}
    Release RelWithDebInfo MinSizeRel Debug NOCONFIG
    )
  if(NOT _cfg)
    continue()
  endif()
  string(TOUPPER "${_cfg}" _cfg_upper)
  get_target_property(_qttesting_imported_location
    ${_qttesting_target} IMPORTED_LOCATION_${_cfg_upper})
  if(_qttesting_imported_location)
    break()
  endif()
endforeach()
if(NOT _qttesting_imported_location)
  get_target_property(_qttesting_imported_location
    ${_qttesting_target} IMPORTED_LOCATION)
endif()
if(NOT _qttesting_imported_location)
  message(FATAL_ERROR
    "SlicerBlockInstallQtTesting: failed to read IMPORTED_LOCATION from "
    "the '${_qttesting_target}' target.")
endif()
get_filename_component(_qttesting_libname "${_qttesting_imported_location}" NAME)

if(WIN32)
  install(FILES "${QtTesting_INSTALL_DIR}/bin/${_qttesting_libname}"
    DESTINATION bin COMPONENT Runtime)
elseif(APPLE)
  install(FILES "${QtTesting_INSTALL_DIR}/lib/${_qttesting_libname}"
    DESTINATION ${QtTesting_INSTALL_LIB_DIR} COMPONENT Runtime)
elseif(UNIX)
  install(FILES "${QtTesting_INSTALL_DIR}/lib/${_qttesting_libname}"
    DESTINATION ${QtTesting_INSTALL_LIB_DIR} COMPONENT Runtime)
  slicerStripInstalledLibrary(
    FILES "${QtTesting_INSTALL_LIB_DIR}/${_qttesting_libname}"
    COMPONENT Runtime)
endif()
