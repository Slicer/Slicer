
# test_cmd .........: command to run without args
# test_name ........: name of the test found in the testing wrapper <test_cmd>
# input_scene ......:
# c{1,2,3,4,5,6} ...: points
# c2 ...............:
# c3 ...............:
# c4 ...............:
# c5 ...............:
# c6 ...............:
# output_model .....: contains the name of the output file the <test_cmd> will produce
# output_baseline ..: contains the name of the "baseline" output file

# Sanity checks
set(expected_defined_vars test_cmd test_name input_scene c1 c2 c3 c4 c5 c6 output_model output_baseline)
foreach(var ${expected_defined_vars})
  if(NOT ${var})
    message(FATAL_ERROR "Variable ${var} not defined !")
  endif()
endforeach()

# Run the test
execute_process(
  COMMAND ${test_cmd} ${test_name}
    --inputScene ${input_scene}
    -c ${c1}  -c ${c2}  -c ${c3}  -c ${c4} -c ${c5} -c ${c6}
    --outputModel ${output_model}
    --is_test 1
  RESULT_VARIABLE result_variable OUTPUT_QUIET
  )

# message(STATUS "result_variable: ${result_variable}" )
if(result_variable EQUAL 2)
  message(SEND_ERROR "${test_cmd} failed with args --inputScene ${input_scene} -c ${c1}  -c ${c2}  -c ${c3}  -c ${c4} -c ${c5} -c ${c6} --outputModel ${output_model} --is_test 1")
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${output_baseline} ${output_model}
  RESULT_VARIABLE test_not_successful
  OUTPUT_QUIET
  ERROR_QUIET
  )

if(test_not_successful)
  message(SEND_ERROR "${output_model} does not match ${output_baseline}!")
endif()
