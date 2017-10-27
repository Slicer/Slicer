
#
# Apply a patch only once
#
# To find out if a patch has already been applied, the script will
# check if a file with the suffix ".applied" exists in build directory
# specified using <BINARY_DIR>.
#
# If DOWNLOAD_STAMP is modified after the patch has been applied, this means that
# the source have been re-downloaded and that the patch must be re-applied.
#
# Usage:
#
#   cmake \
#     -DPATCH:FILEPATH=/path/to/file.patch \
#     -DPatch_EXECUTABLE:PATH=/path/to/patch \
#     -DSOURCE_DIR:PATH=/path/to/src \
#     -DBINARY_DIR:PATH=/path/to/build \
#     -DDOWNLOAD_STAMP:PATH=/path/to/download-stamp \
#     SlicerPatch.cmake
#

# Sanity checks
foreach(varname IN ITEMS
    BINARY_DIR
    Patch_EXECUTABLE
    PATCH
    SOURCE_DIR
  )
  if(NOT EXISTS "${${varname}}")
    message(FATAL_ERROR "Variable ${varname} is set to an inexistent directory or file [${${varname}}]")
  endif()
endforeach()

get_filename_component(patch_filename ${PATCH} NAME)


set(msg "Applying '${patch_filename}'")
set(applied ${BINARY_DIR}/${patch_filename}.applied)

set(reapply 0)
if(EXISTS "${DOWNLOAD_STAMP}" AND "${DOWNLOAD_STAMP}" IS_NEWER_THAN "${applied}")
  set(reapply 1)
endif()

if(EXISTS ${applied})
  if(NOT reapply)
    message(STATUS "${msg} - skipping (already applied)")
    return()
  else()
     file(REMOVE ${applied})
  endif()
endif()
execute_process(
  COMMAND ${Patch_EXECUTABLE} --quiet -p1 -i ${PATCH}
  WORKING_DIRECTORY ${SOURCE_DIR}
  RESULT_VARIABLE result
  ERROR_VARIABLE error
  ERROR_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE output
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
if(result EQUAL 0)
  message(STATUS "${msg} - done")
  execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${applied})
else()
  message(STATUS "${msg} - failed")
  message(FATAL_ERROR "${output}\n${error}")
endif()
