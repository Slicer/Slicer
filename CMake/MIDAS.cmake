# - Configure a project for downloading test data from a MIDAS server
# Include this module in the top CMakeLists.txt file of a project to
# enable downloading test data from MIDAS. Requires CTest module.
#   project(MyProject)
#   ...
#   include(CTest)
#   include(MIDAS)
#
# To use this module, set the following variable in your script:
#   MIDAS_REST_URL - URL of the MIDAS server's REST API
# Other optional variables:
#   MIDAS_DATA_DIR         - Where to place downloaded files
#                          - Defaults to PROJECT_BINARY_DIR/MIDAS_Data
#   MIDAS_KEY_DIR          - Where the key files are located
#                          - Defaults to PROJECT_SOURCE_DIR/MIDAS_Keys
#   MIDAS_DOWNLOAD_TIMEOUT - Timeout for download stage (default 0)
#
# Then call the following macro: 
#  midas_add_test(<testName> <program> [args...])
#   testName: Name of the test
#   program: The executable to be run after the download is complete
#   args: Optional args to the program.  If an arg is of the form
#         MIDAS{foo.ext.md5}, the actual file foo.ext will be
#         substituted for it at test time once the file is downloaded,
#         assuming the keyfile foo.ext.md5 exists in MIDAS_KEY_DIR.
#
# EXAMPLE:
#  midas_add_test(someTest php MIDAS{test.php.md5})
#   is analogous to
#  add_test(someTest php test.php)
#   if the file test.php had already existed on disk.
#
# NOTES:
# * The MIDAS{} substitution method can also be passed a relative path
#   from the MIDAS_KEY_DIR to the key file, so that you can place key files
#   in subdirectories under MIDAS_KEY_DIR. Ex: MIDAS{test1/input/foo.png.md5}
#
#=============================================================================
# Copyright 2010 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

function(midas_add_test)
  # Determine the test name.
  list(GET ARGN 0 firstArg)
  if(firstArg STREQUAL "NAME")
    list(GET ARGN 1 testName)
  else(firstArg STREQUAL "NAME")
    list(GET ARGN 0 testName)
  endif(firstArg STREQUAL "NAME")

  if(NOT DEFINED MIDAS_REST_URL)
    message(FATAL_ERROR "You must set MIDAS_REST_URL to the URL of the MIDAS REST API.")
  endif(NOT DEFINED MIDAS_REST_URL)

  if(NOT DEFINED MIDAS_KEY_DIR)
    set(MIDAS_KEY_DIR "${PROJECT_SOURCE_DIR}/MIDAS_Keys")
  endif(NOT DEFINED MIDAS_KEY_DIR)

  if(NOT DEFINED MIDAS_DATA_DIR)
    set(MIDAS_DATA_DIR "${PROJECT_BINARY_DIR}/MIDAS_Data")
  endif(NOT DEFINED MIDAS_DATA_DIR)
  file(MAKE_DIRECTORY "${MIDAS_DATA_DIR}/MIDAS_FetchScripts")
  file(MAKE_DIRECTORY "${MIDAS_DATA_DIR}/MIDAS_Hashes")

  if(NOT DEFINED MIDAS_DOWNLOAD_TIMEOUT)
    set(MIDAS_DOWNLOAD_TIMEOUT_STR "")
  else(NOT DEFINED MIDAS_DOWNLOAD_TIMEOUT)
    set(MIDAS_DOWNLOAD_TIMEOUT_STR "TIMEOUT ${MIDAS_DOWNLOAD_TIMEOUT}")
  endif(NOT DEFINED MIDAS_DOWNLOAD_TIMEOUT)

  # Substitute the downloaded file argument(s)
  foreach(arg ${ARGN})
    if(arg MATCHES "MIDAS{[^}]*}")
      string(REGEX MATCH "MIDAS{([^}]*)}" toReplace "${arg}")
      string(REGEX REPLACE "MIDAS{([^}]*)}" "\\1" keyFile "${toReplace}")
      # Split up the checksum extension from the real filename
      string(REGEX MATCH "\\.[^\\.]*$" hash_alg "${keyFile}")
      string(REGEX REPLACE "\\.[^\\.]*$" "" base_file "${keyFile}")
      string(REPLACE "." "" hash_alg "${hash_alg}")
      string(TOUPPER "${hash_alg}" hash_alg)
      get_filename_component(base_filepath "${base_file}" PATH)
      get_filename_component(base_filename "${base_file}" NAME)
      get_filename_component(base_fileext  "${base_file}" EXT)

      # Resolve file location
      if(NOT EXISTS "${MIDAS_KEY_DIR}/${keyFile}")
        message(FATAL_ERROR "MIDAS key file ${MIDAS_KEY_DIR}/${keyFile} does not exist.")
      endif(NOT EXISTS "${MIDAS_KEY_DIR}/${keyFile}")

      # Obtain the checksum
      file(READ "${MIDAS_KEY_DIR}/${keyFile}" checksum)

      # Write the test script file for downloading
      file(WRITE "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/fetch_${checksum}_${base_filename}.cmake"
  "message(STATUS \"Data is here: ${MIDAS_REST_URL}/midas.bitstream.by.hash?hash=${checksum}&algorithm=${hash_alg}\")
if(NOT EXISTS \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\")
  file(DOWNLOAD ${MIDAS_REST_URL}/midas.bitstream.by.hash?hash=${checksum}&algorithm=${hash_alg} \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${testName}_${checksum}\" ${MIDAS_DOWNLOAD_TIMEOUT_STR} STATUS status)
  list(GET status 0 exitCode)
  list(GET status 1 errMsg)
  if(NOT exitCode EQUAL 0)
    file(REMOVE \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${testName}_${checksum}\")
    message(FATAL_ERROR \"Error downloading ${checksum}: \${errMsg}\")
  endif(NOT exitCode EQUAL 0)

  execute_process(COMMAND \"${CMAKE_COMMAND}\" -E md5sum \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${testName}_${checksum}\" OUTPUT_VARIABLE output)
  string(SUBSTRING \${output} 0 32 computedChecksum)

  if(NOT computedChecksum STREQUAL ${checksum})
    file(READ \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${testName}_${checksum}\" serverResponse)
    file(REMOVE \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${testName}_${checksum}\")
    message(FATAL_ERROR \"Error: Computed checksum (\${computedChecksum}) did not match expected (${checksum}). Server response: \${serverResponse}\")
  else(NOT computedChecksum STREQUAL ${checksum})
    file(RENAME \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${testName}_${checksum}\" \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\")
  endif(NOT computedChecksum STREQUAL ${checksum})
endif(NOT EXISTS \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\")

# Add a symbolic link so we can use the human-readable filename in the command line
file(MAKE_DIRECTORY \"${MIDAS_DATA_DIR}/${base_filepath}\")
file(REMOVE \"${MIDAS_DATA_DIR}/${base_file}\")

if(WIN32)
  # windows does not support symlinks, so we must duplicate the file for now
  configure_file(\"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\" \"${MIDAS_DATA_DIR}/${base_file}\" COPYONLY)
else(WIN32)
  execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\" \"${MIDAS_DATA_DIR}/${base_file}\")
endif(WIN32)
")

      list(APPEND downloadScripts "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/fetch_${checksum}_${base_filename}.cmake")
      string(REGEX REPLACE ${toReplace} "${MIDAS_DATA_DIR}/${base_file}" newArg "${arg}")
      list(APPEND testArgs ${newArg})
    else(arg MATCHES "MIDAS{[^}]*}")
      list(APPEND testArgs ${arg})
    endif(arg MATCHES "MIDAS{[^}]*}")
  endforeach(arg)

  file(WRITE "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/${testName}_fetchData.cmake"
       "#This is an auto generated file -- do not edit\n\n")
  list(REMOVE_DUPLICATES downloadScripts)
  foreach(downloadScript ${downloadScripts})
    file(APPEND "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/${testName}_fetchData.cmake" "include(\"${downloadScript}\")\n")
  endforeach(downloadScript)

  add_test(${testName}_fetchData "${CMAKE_COMMAND}" -P "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/${testName}_fetchData.cmake")
  set_tests_properties(${testName}_fetchData PROPERTIES FAIL_REGULAR_EXPRESSION "(Error downloading)|(Error: Computed checksum)")
  # Finally, create the test
  add_test(${testArgs})
  set_tests_properties(${testName} PROPERTIES DEPENDS ${testName}_fetchData)
endfunction(midas_add_test)
