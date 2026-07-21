cmake_minimum_required(VERSION 3.28.0...3.28.0 FATAL_ERROR)

if(NOT DEFINED Slicer_SOURCE_DIR)
  message(FATAL_ERROR "Slicer_SOURCE_DIR is required")
endif()
if(NOT DEFINED TEST_BINARY_DIR)
  message(FATAL_ERROR "TEST_BINARY_DIR is required")
endif()

include("${Slicer_SOURCE_DIR}/CMake/SlicerExtensionCPackBundleFixupPath.cmake")

function(assert_path_in_directory candidate directory expected description)
  slicer_extension_cpack_is_path_in_directory("${candidate}" "${directory}" actual)
  set(failed FALSE)
  if(expected AND NOT actual)
    set(failed TRUE)
  elseif(NOT expected AND actual)
    set(failed TRUE)
  endif()
  if(failed)
    message(FATAL_ERROR
      "${description}\n"
      "  candidate: ${candidate}\n"
      "  directory: ${directory}\n"
      "  expected:  ${expected}\n"
      "  actual:    ${actual}")
  endif()
endfunction()

function(assert_slicer_owned item extension_build_dir slicer_superbuild_dir expected description)
  slicer_extension_cpack_item_is_slicer_owned(
    "${item}" "${extension_build_dir}" "${slicer_superbuild_dir}" actual)
  set(failed FALSE)
  if(expected AND NOT actual)
    set(failed TRUE)
  elseif(NOT expected AND actual)
    set(failed TRUE)
  endif()
  if(failed)
    message(FATAL_ERROR
      "${description}\n"
      "  item:                ${item}\n"
      "  extension build dir: ${extension_build_dir}\n"
      "  Slicer build dir:    ${slicer_superbuild_dir}\n"
      "  expected:            ${expected}\n"
      "  actual:              ${actual}")
  endif()
endfunction()

file(REMOVE_RECURSE "${TEST_BINARY_DIR}")

set(physical_dashboard_dir "${TEST_BINARY_DIR}/Users/svc-dashboard/D")
set(physical_slicer_dir "${physical_dashboard_dir}/S/A")
set(logical_dashboard_dir "${TEST_BINARY_DIR}/D")
set(logical_slicer_dir "${logical_dashboard_dir}/S/A")
set(physical_extension_dir "${physical_dashboard_dir}/S/S-0-E-b/MarkupsToModel-build")
set(logical_extension_dir "${logical_dashboard_dir}/S/S-0-E-b/MarkupsToModel-build")
set(physical_nested_extension_dir "${physical_slicer_dir}/E/EmbeddedExtension-build")
set(logical_nested_extension_dir "${logical_slicer_dir}/E/EmbeddedExtension-build")

set(physical_slicer_item "${physical_slicer_dir}/DCMTK-build/lib/libdcmjp2kcs.20.dylib")
set(logical_slicer_item "${logical_slicer_dir}/DCMTK-build/lib/libdcmjp2kcs.20.dylib")
set(physical_extension_item "${physical_extension_dir}/lib/libMarkupsToModel.dylib")
set(logical_extension_item "${logical_extension_dir}/lib/libMarkupsToModel.dylib")
set(physical_nested_extension_item "${physical_nested_extension_dir}/lib/libEmbeddedExtension.dylib")
set(prefix_sibling_item "${physical_dashboard_dir}/S/A-other/lib/libSibling.dylib")
set(external_item "${TEST_BINARY_DIR}/opt/lib/libExternal.dylib")

foreach(item IN ITEMS
    "${physical_slicer_item}"
    "${physical_extension_item}"
    "${physical_nested_extension_item}"
    "${prefix_sibling_item}"
    "${external_item}")
  get_filename_component(item_dir "${item}" DIRECTORY)
  file(MAKE_DIRECTORY "${item_dir}")
  file(TOUCH "${item}")
endforeach()

file(CREATE_LINK "${physical_dashboard_dir}" "${logical_dashboard_dir}" SYMBOLIC RESULT link_result)
if(NOT link_result STREQUAL "0")
  message(FATAL_ERROR "Failed to create dashboard path alias: ${link_result}")
endif()

file(REAL_PATH "${logical_slicer_dir}" canonical_logical_slicer_dir)
file(REAL_PATH "${physical_slicer_dir}" canonical_physical_slicer_dir)
if(NOT canonical_logical_slicer_dir STREQUAL canonical_physical_slicer_dir)
  message(FATAL_ERROR
    "Test fixture does not reproduce the path alias\n"
    "  logical:  ${logical_slicer_dir}\n"
    "  expected: ${canonical_physical_slicer_dir}\n"
    "  actual:   ${canonical_logical_slicer_dir}")
endif()

assert_path_in_directory(
  "${physical_slicer_item}" "${physical_slicer_dir}" TRUE
  "A physical Slicer item must be inside the physical Slicer root")
assert_path_in_directory(
  "${logical_slicer_item}" "${physical_slicer_dir}" TRUE
  "A logical Slicer item must match the equivalent physical Slicer root")
assert_path_in_directory(
  "${physical_slicer_item}" "${logical_slicer_dir}" TRUE
  "A physical Slicer item must match the equivalent logical Slicer root")
assert_path_in_directory(
  "${logical_extension_item}" "${physical_extension_dir}" TRUE
  "A logical extension item must match the equivalent physical extension root")
assert_path_in_directory(
  "${physical_slicer_dir}/../A/DCMTK-build/lib/libdcmjp2kcs.20.dylib"
  "${physical_slicer_dir}" TRUE
  "Parent path components must be normalized")
assert_path_in_directory(
  "${prefix_sibling_item}" "${physical_slicer_dir}" FALSE
  "A similarly named sibling must not be treated as a child")
assert_path_in_directory(
  "@rpath/libdcmjp2kcs.20.dylib" "${physical_slicer_dir}" FALSE
  "A relative load command is not an owned build-tree path")
assert_path_in_directory(
  "${physical_slicer_dir}/missing/libMissing.dylib" "${physical_slicer_dir}" FALSE
  "A missing path must not be classified as owned")

assert_slicer_owned(
  "${logical_slicer_item}" "${physical_extension_dir}" "${physical_slicer_dir}" TRUE
  "A logical Slicer item must be recognized with a physical Slicer root")
assert_slicer_owned(
  "${physical_extension_item}" "${logical_extension_dir}" "${physical_slicer_dir}" FALSE
  "An ordinary extension must retain ownership of its files")
assert_slicer_owned(
  "${physical_nested_extension_item}" "${logical_nested_extension_dir}" "${physical_slicer_dir}" FALSE
  "An extension nested in the Slicer tree must retain ownership across path aliases")
assert_slicer_owned(
  "${external_item}" "${physical_extension_dir}" "${physical_slicer_dir}" FALSE
  "An external library must not be classified as Slicer-owned")
assert_slicer_owned(
  "${prefix_sibling_item}" "${physical_extension_dir}" "${physical_slicer_dir}" FALSE
  "A prefix-collision sibling must not be classified as Slicer-owned")

message("SUCCESS")
