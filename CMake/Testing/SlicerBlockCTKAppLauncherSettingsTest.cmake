cmake_minimum_required(VERSION 3.28.0...3.28.0 FATAL_ERROR)

# Exercise the macOS launcher settings on every host platform.
set(APPLE TRUE)
set(Slicer_USE_SYSTEM_QT TRUE)
set(Slicer_USE_PYTHONQT FALSE)
set(Slicer_BIN_DIR bin)
set(Slicer_LIB_DIR lib/Slicer-test)
set(Slicer_CLIMODULES_LIB_DIR lib/Slicer-test/cli-modules)
set(Slicer_QTLOADABLEMODULES_LIB_DIR lib/Slicer-test/qt-loadable-modules)

include("${CMAKE_CURRENT_LIST_DIR}/../SlicerBlockCTKAppLauncherSettings.cmake")

set(root_library_path "<APPLAUNCHER_SETTINGS_DIR>/../lib")
if(NOT root_library_path IN_LIST SLICER_LIBRARY_PATHS_INSTALLED)
  message(FATAL_ERROR "macOS installed library paths do not contain ${root_library_path}")
endif()

message("SUCCESS")
