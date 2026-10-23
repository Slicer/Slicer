
# test_cmd .........: command to run without args
# test_name ........: name of the test found in the testing wrapper <test_cmd>
# model{1,2} .......:
# output_model .....: name of the output file the <test_cmd> will produce
# output_baseline ..: name of the "baseline" output file

# Sanity checks
set(expected_defined_vars test_cmd test_name model1 model2 output_model output_baseline)
foreach(var ${expected_defined_vars})
  if(NOT ${var})
    message(FATAL_ERROR "Variable ${var} not defined !")
  endif()
endforeach()

# Run the test
execute_process(
  COMMAND ${test_cmd} ${test_name} ${model1} ${model2} ${output_model}
  RESULT_VARIABLE exec_not_successful
  )

if(exec_not_successful)
  message( SEND_ERROR "${test_cmd} failed with args ${test_name} ${model1} ${model2} ${output_model}" )
endif()

# Compare the model content (points, cells, point and cell data) instead of the
# file bytes, because the compressed file content may differ for the same model
# (for example, zlib-ng output depends on the CPU architecture).
execute_process(
  COMMAND ${test_cmd} CompareModels ${output_baseline} ${output_model}
  RESULT_VARIABLE test_not_successful
  )

if(test_not_successful)
  message(SEND_ERROR "${output_model} does not match ${output_baseline}!")
endif()
