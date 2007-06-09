# This script is used to test the output files writtent by a command
# line executable. First, the executable is run, generating a
# temporary file(s).  Second, a temporary file is compared to a known
# standard file.  Finally, the temporary file(s) is deleted.

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.2 FATAL_ERROR)

MESSAGE(STATUS "test_name='${test_name}'")
MESSAGE(STATUS "test_exe='${test_exe}'")
MESSAGE(STATUS "test_exe_args='${test_exe_args}'")
MESSAGE(STATUS "file_cmp='${file_cmp}'")
MESSAGE(STATUS "file_std='${file_std}'")
MESSAGE(STATUS "rm_files='${rm_files}'")

# run the command, generate the temporary file
EXECUTE_PROCESS(
  COMMAND ${test_exe} ${test_exe_args}
  RESULT_VARIABLE test_exe_rv
)
IF(NOT "${test_exe_rv}" STREQUAL "0")
  MESSAGE(FATAL_ERROR "error: '${test_exe}' returned '${test_exe_rv}'")
ENDIF(NOT "${test_exe_rv}" STREQUAL "0")

# diff the temporary file with know correct results
EXECUTE_PROCESS(
  COMMAND "${CMAKE_COMMAND}" -E compare_files "${file_std}" "${file_cmp}"
  RESULT_VARIABLE cmake_compare_rv
  )
IF(NOT "${cmake_compare_rv}" STREQUAL "0")
  MESSAGE(FATAL_ERROR "error: '${CMAKE_COMMAND} -E compare_files' returned '${cmake_compare_rv}'")
ENDIF(NOT "${cmake_compare_rv}" STREQUAL "0") 

# remove the temporary files
IF(NOT "${rm_files}" STREQUAL "")
  EXECUTE_PROCESS(
    COMMAND "${CMAKE_COMMAND}" -E remove ${rm_files}
    RESULT_VARIABLE cmake_remove_rv
    )
  IF(NOT "${cmake_remove_rv}" STREQUAL "0")
    MESSAGE(FATAL_ERROR "error: '${CMAKE_COMMAND} -E remove' returned '${cmake_remove_rv}'")
  ENDIF(NOT "${cmake_remove_rv}" STREQUAL "0")
ENDIF(NOT "${rm_files}" STREQUAL "")  
