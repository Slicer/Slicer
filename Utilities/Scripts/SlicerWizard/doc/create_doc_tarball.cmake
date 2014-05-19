file(GLOB files RELATIVE ${INPUT_PATH} "${INPUT_PATH}/*")
file(GLOB excluded_files RELATIVE ${INPUT_PATH} "${INPUT_PATH}/.*")

foreach(excluded_file ${excluded_files})
  list(REMOVE_ITEM files ${excluded_file})
endforeach()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E remove ${OUTPUT_FILE}
  COMMAND ${CMAKE_COMMAND} -E tar cfz ${OUTPUT_FILE} ${files}
  WORKING_DIRECTORY ${INPUT_PATH}
  RESULT_VARIABLE result
  ERROR_VARIABLE err
  )
if(result)
  message(FATAL_ERROR "Failed to create tarball: ${result}\n${err}")
endif()
