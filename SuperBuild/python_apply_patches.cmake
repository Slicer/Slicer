#
# Apply patches to the python-cmake-buildsystem source tree.
#
# Usage:
#
#   cmake -DGIT_EXECUTABLE:FILEPATH=/path/to/git
#         -DSOURCE_DIR:PATH=/path/to/python-cmake-buildsystem
#         -DPATCH_DIR:PATH=/path/to/directory/with/patches
#         -P python_apply_patches.cmake
#
# All "*.patch" files found in PATCH_DIR are applied in alphabetical order
# using "git apply". A patch already applied (for example after re-running
# the patch step) is skipped.
#

foreach(varname IN ITEMS GIT_EXECUTABLE SOURCE_DIR PATCH_DIR)
  if(NOT DEFINED ${varname})
    message(FATAL_ERROR "${varname} is not defined")
  endif()
endforeach()

file(GLOB patches LIST_DIRECTORIES false "${PATCH_DIR}/*.patch")
list(SORT patches)

foreach(patch IN LISTS patches)
  get_filename_component(patch_name ${patch} NAME)

  execute_process(
    COMMAND ${GIT_EXECUTABLE} apply --reverse --check ${patch}
    WORKING_DIRECTORY ${SOURCE_DIR}
    RESULT_VARIABLE already_applied_result
    OUTPUT_QUIET
    ERROR_QUIET
    )
  if(already_applied_result EQUAL 0)
    message(STATUS "Skipping already applied patch: ${patch_name}")
    continue()
  endif()

  message(STATUS "Applying patch: ${patch_name}")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} apply --whitespace=nowarn ${patch}
    WORKING_DIRECTORY ${SOURCE_DIR}
    RESULT_VARIABLE apply_result
    ERROR_VARIABLE apply_error
    )
  if(NOT apply_result EQUAL 0)
    message(FATAL_ERROR "Failed to apply patch ${patch} in ${SOURCE_DIR}:\n${apply_error}")
  endif()
endforeach()
