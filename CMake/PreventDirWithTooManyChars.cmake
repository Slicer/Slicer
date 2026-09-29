#
# This function will prevent building the project from a source or build directory having
# too many characters.
#
# The maximum path length should be specified setting the variable ${PROJECT_NAME}_ROOT_DIR_MAX_LENGTH.
# A separate binary directory limit may be set with ${PROJECT_NAME}_BUILD_DIR_MAX_LENGTH.
# For example:
#
#   set(${PROJECT_NAME}_ROOT_DIR_MAX_LENGTH 40)
#
# To skip the directory length check, the project can be configured with:
#
#   -D<PROJECT_NAME>_SKIP_ROOT_DIR_MAX_LENGTH_CHECK:BOOL=TRUE
#
function(AssureLengthForSourceOrBuildDir max_length)

  if(ARGC GREATER 1)
    set(binary_max_length "${ARGV1}")
  else()
    set(binary_max_length "${max_length}")
  endif()

  function(_check_path_length path description max_length)
    string(LENGTH "${path}" n)
    if(n GREATER ${max_length})
      string(SUBSTRING "${path}" 0 ${max_length} _expected_path)
      message(
        FATAL_ERROR
        "The current ${description} directory has too many characters:\n"
        "  current path:\n"
        "    ${path} [${n} chars]\n"
        "  expected path:\n"
        "    ${_expected_path} [${max_length} chars]\n"
        ""
        "Please use a shorter directory for ${PROJECT_NAME} ${description} directory.\n"
        ""
        "To ignore this error, reconfigure with the following option:\n"
        "  -D${PROJECT_NAME}_SKIP_ROOT_DIR_MAX_LENGTH_CHECK:BOOL=TRUE\n"
        )
    endif()
  endfunction()

  _check_path_length("${CMAKE_CURRENT_SOURCE_DIR}" "source" "${max_length}")
  _check_path_length("${CMAKE_CURRENT_BINARY_DIR}" "binary" "${binary_max_length}")

endfunction()

if(NOT DEFINED ${PROJECT_NAME}_SKIP_ROOT_DIR_MAX_LENGTH_CHECK)
  set(${PROJECT_NAME}_SKIP_ROOT_DIR_MAX_LENGTH_CHECK FALSE)
endif()
if(NOT ${PROJECT_NAME}_SKIP_ROOT_DIR_MAX_LENGTH_CHECK)
  if("${${PROJECT_NAME}_ROOT_DIR_MAX_LENGTH}" STREQUAL "")
    message(FATAL_ERROR "Variable ${PROJECT_NAME}_ROOT_DIR_MAX_LENGTH should be set to an integer value > 0.")
  endif()
  if(DEFINED ${PROJECT_NAME}_BUILD_DIR_MAX_LENGTH
      AND "${${PROJECT_NAME}_BUILD_DIR_MAX_LENGTH}" STREQUAL "")
    message(FATAL_ERROR "Variable ${PROJECT_NAME}_BUILD_DIR_MAX_LENGTH should be set to an integer value > 0.")
  endif()
  AssureLengthForSourceOrBuildDir(
    ${${PROJECT_NAME}_ROOT_DIR_MAX_LENGTH}
    ${${PROJECT_NAME}_BUILD_DIR_MAX_LENGTH})
endif()
