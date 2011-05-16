
# arguments checking
if( NOT TEST_PROGRAM )
  message( FATAL_ERROR "Require TEST_PROGRAM to be defined" )
endif( NOT TEST_PROGRAM )
if( NOT TEST_COMPARE_PROGRAM )
  message( FATAL_ERROR "Require TEST_COMPARE_PROGRAM to be defined" )
endif( NOT TEST_COMPARE_PROGRAM )
if( NOT TEST_BASELINE )
  message( FATAL_ERROR "Require TEST_BASELINE to be defined" )
endif( NOT TEST_BASELINE )
if( NOT TEST_INPUT )
  message( FATAL_ERROR "Require TEST_INPUT to be defined" )
endif( NOT TEST_INPUT )
if( NOT TEST_TEMP_OUTPUT )
  message( FATAL_ERROR "Require TEST_TEMP_OUTPUT to be defined" )
endif( NOT TEST_TEMP_OUTPUT )

# Run the compare program to make sure it built correctly
execute_process(
  COMMAND ${TEST_COMPARE_PROGRAM} --help
  RESULT_VARIABLE TEST_RESULT
  )

# if the return value is !=0 bail out
if( TEST_RESULT )
  message( FATAL_ERROR "Failed: Test compare program ${TEST_COMPARE_PROGRAM} won't run.\n${TEST_ERROR}" )
endif( TEST_RESULT )

# Check to see if the image we are comparing against exists.  We do this here to avoid a lengthy test for no reason.
if(NOT EXISTS ${TEST_BASELINE})
  message( FATAL_ERROR "Failed: Baseline image ${TEST_BASELINE} does not exist!\n") 
endif( NOT EXISTS ${TEST_BASELINE})

# run the test program, capture the stdout/stderr and the result var
execute_process(
  COMMAND ${TEST_PROGRAM} --inputDicomDirectory ${TEST_INPUT} --outputVolume ${TEST_TEMP_OUTPUT} ${TEST_PROGRAM_ARGS}
  ERROR_VARIABLE TEST_ERROR
  RESULT_VARIABLE TEST_RESULT
  )

# if the return value is !=0 bail out
if( TEST_RESULT )
  message( FATAL_ERROR "Failed: Test program ${TEST_PROGRAM} exited != 0.\n${TEST_ERROR}" )
endif( TEST_RESULT )

# now compare the output with the reference
execute_process(
  COMMAND ${TEST_COMPARE_PROGRAM} --inputVolume2 ${TEST_TEMP_OUTPUT} --inputVolume1 ${TEST_BASELINE}
  RESULT_VARIABLE TEST_RESULT
  )

# again, if return value is !=0 scream and shout
if( TEST_RESULT )
  message( FATAL_ERROR "Failed: The output of ${TEST_PROGRAM} did not match ${TEST_BASELINE}: ${TEST_RESULT}")
endif( TEST_RESULT )

# everything went fine...
message( "Passed: The output of ${TEST_PROGRAM} matches ${TEST_BASELINE}" )

