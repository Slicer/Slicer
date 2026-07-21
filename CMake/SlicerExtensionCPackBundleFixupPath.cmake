cmake_minimum_required(VERSION 3.28.0...3.28.0 FATAL_ERROR)

# Return whether an existing absolute candidate path is contained by an
# existing absolute directory. Resolve path aliases and normalize path
# components before comparing them.
function(slicer_extension_cpack_is_path_in_directory candidate directory output_var)
  set(is_in_directory FALSE)
  if(IS_ABSOLUTE "${candidate}"
      AND IS_ABSOLUTE "${directory}"
      AND EXISTS "${candidate}"
      AND IS_DIRECTORY "${directory}")
    file(REAL_PATH "${candidate}" real_candidate)
    file(REAL_PATH "${directory}" real_directory)
    cmake_path(IS_PREFIX real_directory "${real_candidate}" NORMALIZE is_in_directory)
  endif()
  set(${output_var} ${is_in_directory} PARENT_SCOPE)
endfunction()

# Return whether an item is owned by the Slicer build. An extension built
# inside the Slicer tree continues to own files from its own build directory.
# This supports custom applications (e.g., SlicerSALT) that cannot bundle a
# specific extension through Slicer_EXTENSION_SOURCE_DIRS and instead package
# it separately and copy its contents into the custom application package.
function(slicer_extension_cpack_item_is_slicer_owned
    item extension_build_dir slicer_superbuild_dir output_var)
  slicer_extension_cpack_is_path_in_directory(
    "${item}" "${slicer_superbuild_dir}" item_in_slicer_build_dir)
  slicer_extension_cpack_is_path_in_directory(
    "${extension_build_dir}" "${slicer_superbuild_dir}" extension_built_within_slicer_build_dir)
  slicer_extension_cpack_is_path_in_directory(
    "${item}" "${extension_build_dir}" item_in_extension_build_dir)

  set(is_slicer_owned FALSE)
  if(item_in_slicer_build_dir)
    if(NOT extension_built_within_slicer_build_dir OR NOT item_in_extension_build_dir)
      set(is_slicer_owned TRUE)
    endif()
  endif()
  set(${output_var} ${is_slicer_owned} PARENT_SCOPE)
endfunction()
