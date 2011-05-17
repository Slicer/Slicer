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
#         If an arg is of the form MIDAS_FETCH_ONLY{foo.ext.md5},
#         it will be downloaded prior to running the test, but it will
#         not be passed as an argument to the test.
#
# EXAMPLE:
#  midas_add_test(someTest php MIDAS{test.php.md5})
#   is analogous to
#  add_test(someTest php test.php)
#   if the file test.php had already existed on disk.
#
# NOTES:
# * The MIDAS*{} substitution methods can also be passed a relative path
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

  set(downloadScripts "")

  # Substitute the downloaded file argument(s)
  foreach(arg ${ARGN})
    if(NOT arg MATCHES "MIDAS[a-zA-Z_]*{[^}]*}")
      list(APPEND testArgs ${arg})
    else()
      while(arg MATCHES "MIDAS[a-zA-Z_]*{[^}]*}")
        string(REGEX MATCH "MIDAS[a-zA-Z_]*{([^}]*)}" toReplace "${arg}")
        string(REGEX MATCH "^MIDAS[a-zA-Z_]*" keyword "${toReplace}")
        string(REGEX REPLACE "MIDAS[a-zA-Z_]*{([^}]*)}" "\\1" parameter "${toReplace}")
        if(keyword STREQUAL "MIDAS_DIRECTORY")
          file(GLOB fileList RELATIVE "${MIDAS_KEY_DIR}" "${MIDAS_KEY_DIR}/${parameter}/*")
          foreach(keyFile ${fileList})
            if(NOT IS_DIRECTORY "${MIDAS_KEY_DIR}/${keyFile}")
              _process_keyfile("${keyFile}" ${testName} 0)
            endif(NOT IS_DIRECTORY "${MIDAS_KEY_DIR}/${keyFile}")
          endforeach(keyFile ${fileList})
        elseif(keyword STREQUAL "MIDAS_TGZ")
          _process_keyfile("${parameter}" ${testName} 1)
        else()
          _process_keyfile("${parameter}" ${testName} 0)
        endif(keyword STREQUAL "MIDAS_DIRECTORY")

        if(keyword STREQUAL "MIDAS_DIRECTORY")
          string(REGEX REPLACE ${toReplace} "${MIDAS_DATA_DIR}/${parameter}" newArg "${arg}")
        elseif(keyword STREQUAL "MIDAS_TGZ")
          get_filename_component(extractedPath "${parameter}" PATH)
          get_filename_component(dirName "${parameter}" NAME_WE)
          string(REGEX REPLACE ${toReplace} "${MIDAS_DATA_DIR}/${extractedPath}/${dirName}" newArg "${arg}")
        elseif(NOT keyword STREQUAL "MIDAS_FETCH_ONLY")
          string(REGEX REPLACE ${toReplace} "${MIDAS_DATA_DIR}/${base_file}" newArg "${arg}")
        endif(keyword STREQUAL "MIDAS_DIRECTORY")

        set(arg ${newArg})
      endwhile(arg MATCHES "MIDAS[a-zA-Z_]*{[^}]*}")

      if(NOT keyword STREQUAL "MIDAS_FETCH_ONLY")
        list(APPEND testArgs ${newArg})
      endif(NOT keyword STREQUAL "MIDAS_FETCH_ONLY")
    endif(NOT arg MATCHES "MIDAS[a-zA-Z_]*{[^}]*}")
  endforeach(arg)

  file(WRITE "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/${testName}_fetchData.cmake"
# Start file content
"#This is an auto generated file -- do not edit

set(midas_test_name \"${testName}\")
")
# End file content

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

# Helper macro to write the download scripts for MIDAS.*{} arguments
macro(_process_keyfile keyFile testName extractTgz)
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
  if(UNIX)
    set(cmake_symlink create_symlink)
  else()
    set(cmake_symlink copy) # Windows has no symlinks; copy instead.
  endif()
  file(WRITE "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/fetch_${checksum}_${base_filename}.cmake"
# Start file content
"message(STATUS \"Data is here: ${MIDAS_REST_URL}/midas.bitstream.by.hash?hash=${checksum}&algorithm=${hash_alg}\")
if(NOT EXISTS \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\")
  file(DOWNLOAD \"${MIDAS_REST_URL}/midas.bitstream.by.hash?hash=${checksum}&algorithm=${hash_alg}\" \"${MIDAS_DATA_DIR}/MIDAS_Hashes/\${midas_test_name}_${checksum}\" ${MIDAS_DOWNLOAD_TIMEOUT_STR} STATUS status)
  list(GET status 0 exitCode)
  list(GET status 1 errMsg)
  if(NOT exitCode EQUAL 0)
    file(REMOVE \"${MIDAS_DATA_DIR}/MIDAS_Hashes/\${midas_test_name}_${checksum}\")
    message(FATAL_ERROR \"Error downloading ${checksum}: \${errMsg}\")
  endif(NOT exitCode EQUAL 0)

  execute_process(COMMAND \"${CMAKE_COMMAND}\" -E md5sum \"${MIDAS_DATA_DIR}/MIDAS_Hashes/\${midas_test_name}_${checksum}\" OUTPUT_VARIABLE output)
  string(SUBSTRING \${output} 0 32 computedChecksum)

  if(NOT computedChecksum STREQUAL ${checksum})
    file(READ \"${MIDAS_DATA_DIR}/MIDAS_Hashes/\${midas_test_name}_${checksum}\" serverResponse)
    file(REMOVE \"${MIDAS_DATA_DIR}/MIDAS_Hashes/\${midas_test_name}_${checksum}\")
    message(FATAL_ERROR \"Error: Computed checksum (\${computedChecksum}) did not match expected (${checksum}). Server response: \${serverResponse}\")
  else(NOT computedChecksum STREQUAL ${checksum})
    file(RENAME \"${MIDAS_DATA_DIR}/MIDAS_Hashes/\${midas_test_name}_${checksum}\" \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\")
  endif(NOT computedChecksum STREQUAL ${checksum})
endif(NOT EXISTS \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\")

file(MAKE_DIRECTORY \"${MIDAS_DATA_DIR}/${base_filepath}\")
file(MAKE_DIRECTORY \"${MIDAS_DATA_DIR}/\${midas_test_name}_${base_filepath}\")
")
# End file content

  if(${extractTgz})
    file(APPEND "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/fetch_${checksum}_${base_filename}.cmake"
# Start file content
"# Extract the contents of the tgz
get_filename_component(dirName \"${base_filename}\" NAME_WE)
file(MAKE_DIRECTORY \"${MIDAS_DATA_DIR}/\${midas_test_name}_${base_filepath}/\${dirName}\")
execute_process(COMMAND \"${CMAKE_COMMAND}\" -E tar xzf \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\"
                WORKING_DIRECTORY \"${MIDAS_DATA_DIR}/\${midas_test_name}_${base_filepath}/\${dirName}\")
file(REMOVE_RECURSE \"${MIDAS_DATA_DIR}/${base_filepath}/\${dirName}\")
file(RENAME \"${MIDAS_DATA_DIR}/\${midas_test_name}_${base_filepath}/\${dirName}\" \"${MIDAS_DATA_DIR}/${base_filepath}/\${dirName}\")
")
# End file content
  else()
    file(APPEND "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/fetch_${checksum}_${base_filename}.cmake"
# Start file content
"# Create a human-readable file name for the data.
file(REMOVE \"${MIDAS_DATA_DIR}/\${midas_test_name}_${base_file}\")
execute_process(COMMAND \"${CMAKE_COMMAND}\" -E ${cmake_symlink} \"${MIDAS_DATA_DIR}/MIDAS_Hashes/${checksum}\" \"${MIDAS_DATA_DIR}/\${midas_test_name}_${base_file}\" WORKING_DIRECTORY \"${MIDAS_DATA_DIR}\")
file(RENAME \"${MIDAS_DATA_DIR}/\${midas_test_name}_${base_file}\" \"${MIDAS_DATA_DIR}/${base_file}\")
")
# End file content
  endif(${extractTgz})

  list(APPEND downloadScripts "${MIDAS_DATA_DIR}/MIDAS_FetchScripts/fetch_${checksum}_${base_filename}.cmake")
endmacro(_process_keyfile)
