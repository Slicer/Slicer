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
if (NOT input_scene)
  message ( FATAL_ERROR "Variable input_scene not defined" )
endif (NOT input_scene)

#  points
if (NOT c1)
   message ( FATAL_ERROR "Variable c1 not defined" )
endif (NOT c1)
if (NOT c2)
   message ( FATAL_ERROR "Variable c2 not defined" )
endif (NOT c2)
if (NOT c3)
   message ( FATAL_ERROR "Variable c3 not defined" )
endif (NOT c3)
if (NOT c4)
   message ( FATAL_ERROR "Variable c4 not defined" )
endif (NOT c4)
if (NOT c5)
   message ( FATAL_ERROR "Variable c5 not defined" )
endif (NOT c5)
if (NOT c6)
   message ( FATAL_ERROR "Variable c6 not defined" )
endif (NOT c6)

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

# need two execute process calls since chaining them fails (broken pipe?) and the comparison is run w/o the test having completed

execute_process(
 COMMAND ${test_cmd} ${test_name} --inputScene ${input_scene} -c ${c1}  -c ${c2}  -c ${c3}  -c ${c4} -c ${c5} -c ${c6} --outputModel ${output_model} --is_test 1
 RESULT_VARIABLE sfls_result_out OUTPUT_QUIET
)
message (STATUS "value of output var: ${sfls_result_out}" )
if ( sfls_result_out EQUAL 2)
 message (STATUS "returned value 2 from sfls test" )
endif ( sfls_result_out EQUAL 2)

if ( sfls_result_out EQUAL 0 )
 message (SEND_ERROR "${test_cmd} failed to run properly with args:\n${test_name} --inputScene ${input_scene} -c ${c1}  -c ${c2}  -c ${c3}  -c ${c4} -c ${c5} -c ${c6} --outputModel ${output_model}")
endif ( sfls_result_out EQUAL 0 )

#execute_process(
# COMMAND ${CMAKE_COMMAND} -E compare_files ${output_baseline} ${output_test}
# RESULT_VARIABLE test_not_successful
# OUTPUT_QUIET
# ERROR_QUIET
# )
set( test_not_successful 0 )
if( sfls_result_out EQUAL 1 )
 set( test_not_successful 1 )
 message( SEND_ERROR "${output_test} does not match ${output_baseline}!" )
endif( sfls_result_out EQUAL 1 )
#message( STATUS "value of test_not_succesful: ${test_not_successful}" )
