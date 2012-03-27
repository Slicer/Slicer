
#
# Make sure this CMake has BundleUtilities.cmake:
#
if(NOT EXISTS "${CMAKE_ROOT}/Modules/BundleUtilities.cmake")
  message(FATAL_ERROR "error: BundleUtilities.cmake not found. Use CMake 2.8.3 or later.")
endif()
include(BundleUtilities)

#
# Avoid following symlinks encountered during FILE GLOB_RECURSE calls:
#
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0009 NEW)
  if(POLICY CMP0011)
    cmake_policy(SET CMP0011 NEW)
  endif()
endif()


# Sanity checks
set(expected_existing_vars Slicer_INSTALL_DIR)
foreach(var ${expected_existing_vars})
  if(NOT EXISTS "${MY_${var}}")
    message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
  endif()
endforeach()

verify_app("${Slicer_INSTALL_DIR}/Slicer.app")
