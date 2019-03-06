cmake_minimum_required(VERSION 3.13.4)

foreach(varname IN ITEMS
    CTKAppLauncher_DIR
    LSB_RELEASE_EXECUTABLE
    PYTHON_REAL_EXECUTABLE
    )
  if("${${varname}}" STREQUAL "")
    message(FATAL_ERROR "${varname} is empty")
  endif()
endforeach()

find_package(CTKAppLauncher REQUIRED)

get_filename_component(python_bin_dir "${PYTHON_REAL_EXECUTABLE}" PATH)

# Configure wrapper for lsb_release to support pip installation
# of packages on system where "lsb_release" is it self implemented
# in python.
ctkAppLauncherConfigureForExecutable(
  LAUNCHER_LOAD_ENVIRONMENT 0
  APPLICATION_NAME lsb_release
  SPLASHSCREEN_DISABLED
  # Launcher settings specific to build tree
  APPLICATION_EXECUTABLE ${LSB_RELEASE_EXECUTABLE}
  DESTINATION_DIR ${python_bin_dir}
  )
