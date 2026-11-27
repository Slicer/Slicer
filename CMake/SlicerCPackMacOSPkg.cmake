# Prepare the staged bundle's installer metadata, scripts and license.
# CPack's productbuild generator creates the packages after this hook runs.
set(_stage_dir "${CPACK_TEMPORARY_DIRECTORY}/pkg")
set(_applications_dir "${_stage_dir}/Applications")
file(GLOB _apps LIST_DIRECTORIES true "${_applications_dir}/*.app")
# Keep the glob result as a string so semicolons in application names remain literal.
if(NOT IS_DIRECTORY "${_apps}")
  message(FATAL_ERROR "Expected exactly one application bundle directory in ${_applications_dir}")
endif()
set(_app "${_apps}")
file(GLOB _stage_entries LIST_DIRECTORIES true "${_stage_dir}/*")
file(GLOB _application_entries LIST_DIRECTORIES true "${_applications_dir}/*")
if(NOT "${_stage_entries}" STREQUAL "${_applications_dir}" OR NOT "${_application_entries}" STREQUAL "${_app}")
  message(FATAL_ERROR "Unexpected files outside the application bundle in ${_stage_dir}")
endif()

get_filename_component(_license_extension "${CPACK_RESOURCE_FILE_LICENSE}" LAST_EXT)
string(TOLOWER "${_license_extension}" _license_extension)
if(NOT _license_extension MATCHES "^\\.(txt|rtf|html)$")
  message(FATAL_ERROR "The installer license must be a .txt, .rtf or .html file")
endif()

get_filename_component(_app_name "${_app}" NAME)
get_filename_component(_work_dir "${CPACK_COMPONENT_PKG_PLIST}" DIRECTORY)
file(MAKE_DIRECTORY "${_work_dir}")
if(DEFINED CPACK_COMMAND_PKGBUILD AND NOT "${CPACK_COMMAND_PKGBUILD}" STREQUAL "")
  set(_pkgbuild "${CPACK_COMMAND_PKGBUILD}")
else()
  find_program(_pkgbuild pkgbuild REQUIRED)
endif()
execute_process(
  COMMAND "${_pkgbuild}" --analyze --root "${_stage_dir}" "${CPACK_COMPONENT_PKG_PLIST}"
  COMMAND_ERROR_IS_FATAL ANY
  )

# Read the analyzed plist as JSON to locate the application, then edit only its
# two settings in the original plist so nested bundle metadata is preserved.
execute_process(
  COMMAND /usr/bin/plutil -convert json -o - "${CPACK_COMPONENT_PKG_PLIST}"
  OUTPUT_VARIABLE _components
  COMMAND_ERROR_IS_FATAL ANY
  )
string(JSON _component_count LENGTH "${_components}")
set(_app_index "")
if(_component_count GREATER 0)
  math(EXPR _last_component "${_component_count} - 1")
  foreach(_index RANGE 0 ${_last_component})
    string(JSON _path ERROR_VARIABLE _path_error GET "${_components}" ${_index} RootRelativeBundlePath)
    if(NOT _path_error AND _path STREQUAL "Applications/${_app_name}")
      if(NOT _app_index STREQUAL "")
        message(FATAL_ERROR "pkgbuild recognized the application bundle more than once: ${_app_name}")
      endif()
      set(_app_index "${_index}")
    endif()
  endforeach()
endif()
if(_app_index STREQUAL "")
  message(FATAL_ERROR "pkgbuild did not recognize the application bundle: Applications/${_app_name}")
endif()
foreach(_key BundleIsRelocatable BundleOverwriteAction)
  string(JSON _type ERROR_VARIABLE _key_error TYPE "${_components}" ${_app_index} ${_key})
  if(_key_error)
    set(_operation -insert)
  else()
    set(_operation -replace)
  endif()
  if(_key STREQUAL "BundleIsRelocatable")
    set(_value_type -bool)
    set(_value NO)
  else()
    set(_value_type -string)
    set(_value upgrade)
  endif()
  execute_process(
    COMMAND /usr/bin/plutil ${_operation} "${_app_index}.${_key}" ${_value_type} "${_value}" "${CPACK_COMPONENT_PKG_PLIST}"
    COMMAND_ERROR_IS_FATAL ANY
    )
endforeach()

get_filename_component(_postinstall_dir "${CPACK_POSTFLIGHT_PKG_SCRIPT}" DIRECTORY)
file(MAKE_DIRECTORY "${_postinstall_dir}")
set(_script_dir "${CMAKE_CURRENT_LIST_DIR}/SlicerCPackMacOSPkg")
file(READ "${CPACK_PREFLIGHT_PKG_SCRIPT}" _slicer_pkg_preinstall_script)
string(REPLACE "'" "'\\''" _slicer_pkg_app_name "${_app_name}")
set(_slicer_pkg_app_name "'${_slicer_pkg_app_name}'")
configure_file("${_script_dir}/Postinstall.sh.in" "${CPACK_POSTFLIGHT_PKG_SCRIPT}" @ONLY)
file(CHMOD "${CPACK_POSTFLIGHT_PKG_SCRIPT}"
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# CPack configures its license resource as a template. Use an unmodified copy
# for the distribution and an empty placeholder for CPack's resource handling.
set(_resources "${CPACK_TOPLEVEL_DIRECTORY}/SlicerMacOSPkgResources")
file(MAKE_DIRECTORY "${_resources}")
set(_slicer_pkg_license_filename "SlicerLicense${_license_extension}")
file(COPY_FILE "${CPACK_RESOURCE_FILE_LICENSE}" "${_resources}/${_slicer_pkg_license_filename}")
file(WRITE "${_resources}/Placeholder.txt" "")
set(CPACK_PRODUCTBUILD_RESOURCES_DIR "${_resources}")
set(CPACK_RESOURCE_FILE_LICENSE "${_resources}/Placeholder.txt")
string(REPLACE "&" "&amp;" _slicer_pkg_title_xml "${CPACK_COMPONENT_PKG_DISPLAY_NAME}")
string(REPLACE "<" "&lt;" _slicer_pkg_title_xml "${_slicer_pkg_title_xml}")
string(REPLACE ">" "&gt;" _slicer_pkg_title_xml "${_slicer_pkg_title_xml}")
