
include(BundleUtilities)

# Sanity checks
set(expected_existing_vars Slicer_INSTALL_DIR Slicer_MAIN_PROJECT_APPLICATION_NAME)
foreach(var ${expected_existing_vars})
  if(NOT EXISTS "${MY_${var}}")
    message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
  endif()
endforeach()

verify_app("${Slicer_INSTALL_DIR}/${Slicer_MAIN_PROJECT_APPLICATION_NAME}.app")
