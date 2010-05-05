# some argument checking:
# test_cmd is the command to run without args
if( NOT test_cmd )
 message( FATAL_ERROR "Variable test_cmd not defined" )
endif( NOT test_cmd )

# test_name is the name of the test found in the testing wrapper test_cmd
if (NOT test_name)
  message( FATAL_ERROR "Variable test_name not defined" )
endif( NOT test_name)

# need all the args separately or else they're not passed properly to the command below
if (NOT model1)
message ( FATAL_ERROR "Variable model1 not defined" )
endif (NOT model1)
if (NOT model2)
message ( FATAL_ERROR "Variable model2 not defined" )
endif (NOT model2)

if (NOT output_model)
   message ( FATAL_ERROR "Variable output_model not defined" )
endif (NOT output_model)

# output_baseline contains the name of the "baseline" output file
if( NOT output_baseline )
 message( FATAL_ERROR "Variable output_baseline not defined" )
endif( NOT output_baseline )

# output_test contains the name of the output file the test_cmd will produce
if( NOT output_test )
 message( FATAL_ERROR "Variable output_test not defined" )
endif( NOT output_test )


# now run the test
execute_process(
 COMMAND ${test_cmd} ${test_name} ${model1} ${model2} ${output_model}
 RESULT_VARIABLE exec_not_successful
)

if (exec_not_successful)
 message( SEND_ERROR "${test_cmd} failed with args ${test_name} ${model1} ${model2} ${output_model}" )
endif( exec_not_successful )

execute_process(
 COMMAND ${CMAKE_COMMAND} -E compare_files ${output_baseline} ${output_test}
 RESULT_VARIABLE test_not_successful
 OUTPUT_QUIET
 ERROR_QUIET
 )

if( test_not_successful )
 message( SEND_ERROR "${output_test} does not match ${output_baseline}!" )
endif( test_not_successful )
