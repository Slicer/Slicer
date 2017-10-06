
include(BundleUtilities)

# Sanity checks
set(expected_existing_vars
  Slicer_INSTALL_DIR
  Slicer_MAIN_PROJECT_APPLICATION_NAME
  Slicer_VERSION_FULL
  )
foreach(var ${expected_existing_vars})
  if(NOT EXISTS "${MY_${var}}")
    message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
  endif()
endforeach()

set(app_name "${Slicer_MAIN_PROJECT_APPLICATION_NAME}.app")
verify_app("${Slicer_INSTALL_DIR}/${app_name")
