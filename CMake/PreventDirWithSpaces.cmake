#
# This function will prevent building the project from a source or build directory having spaces
#
# To allow directory with spaces, the project can be configured with:
#
#   -D<PROJECT_NAME>_SKIP_DIR_WITH_SPACES_CHECK:BOOL=TRUE
#
function(AssureNoSpacesForSourceOrBuildDir)

  function(_check_path path description)

    # make sure the user doesn't play dirty with symlinks
    get_filename_component(real_path "${path}" REALPATH)

    string(FIND ${path} " " real_path_space_position)
    set(real_path_contains_spaces FALSE)
    if(NOT real_path_space_position EQUAL -1)
      set(real_path_contains_spaces TRUE)
    endif()

    if(real_path_contains_spaces)
      message(FATAL_ERROR
        "The current ${description} directory contains spaces:\n"
        "  ${real_path}\n"
        "Building the project from a source or build directory having spaces is NOT recommended or tested !\n"
        ""
        "To ignore this error, reconfigure with the following option:\n"
        "  -D${PROJECT_NAME}_SKIP_DIR_WITH_SPACES_CHECK:BOOL=TRUE\n"
        )
    endif()

  endfunction()

  _check_path("${CMAKE_SOURCE_DIR}" "source")
  _check_path("${CMAKE_BINARY_DIR}" "binary")

endfunction()

if(NOT DEFINED ${PROJECT_NAME}_SKIP_DIR_WITH_SPACES_CHECK)
  set(${PROJECT_NAME}_SKIP_DIR_WITH_SPACES_CHECK FALSE)
endif()
if(NOT ${PROJECT_NAME}_SKIP_DIR_WITH_SPACES_CHECK)
  AssureNoSpacesForSourceOrBuildDir()
endif()
