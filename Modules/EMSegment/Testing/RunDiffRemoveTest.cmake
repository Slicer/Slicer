# This script is used to test the output files writtent by a command
# line executable. First, the executable is run, generating a
# temporary file(s).  Second, a temporary file is compared to a known
# standard file.  Finally, the temporary file(s) is deleted.

cmake_minimum_required(VERSION 2.4.2 FATAL_ERROR)

message(STATUS "test_name='${test_name}'")
message(STATUS "test_exe='${test_exe}'")
message(STATUS "test_exe_args='${test_exe_args}'")
message(STATUS "file_cmp='${file_cmp}'")
message(STATUS "file_std='${file_std}'")
message(STATUS "rm_files='${rm_files}'")

# run the command, generate the temporary file
message("Command: ${test_exe} ${test_exe_args}")
execute_process(
  COMMAND ${test_exe} ${test_exe_args}
  RESULT_VARIABLE test_exe_rv
  )
if(NOT "${test_exe_rv}" STREQUAL "0")
  message(FATAL_ERROR "error: '${test_exe}' returned '${test_exe_rv}'")
endif(NOT "${test_exe_rv}" STREQUAL "0")

# diff the temporary file with know correct results
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E compare_files "${file_std}" "${file_cmp}"
  RESULT_VARIABLE cmake_compare_rv
  )
if(NOT "${cmake_compare_rv}" STREQUAL "0")
  message(FATAL_ERROR "error: '${CMAKE_COMMAND} -E compare_files' returned '${cmake_compare_rv}'")
endif(NOT "${cmake_compare_rv}" STREQUAL "0") 

# remove the temporary files
if(NOT "${rm_files}" STREQUAL "")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E remove ${rm_files}
    RESULT_VARIABLE cmake_remove_rv
    )
  if(NOT "${cmake_remove_rv}" STREQUAL "0")
    message(FATAL_ERROR "error: '${CMAKE_COMMAND} -E remove' returned '${cmake_remove_rv}'")
  endif(NOT "${cmake_remove_rv}" STREQUAL "0")
endif(NOT "${rm_files}" STREQUAL "")  
