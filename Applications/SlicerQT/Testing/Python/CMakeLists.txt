SET(TEST_SCRIPTS
  
  )

FOREACH(script ${TEST_SCRIPTS})
  get_filename_component(test_name ${script} NAME_WE)
  ADD_TEST(
    NAME py_${test_name} 
    COMMAND ${LAUNCH_EXE} $<TARGET_FILE:SlicerQT>
    --python-script ${CMAKE_CURRENT_SOURCE_DIR}/${script}
    # TODO Add script arguments
    --exit
    )
  SET_TESTS_PROPERTIES(py_${test_name} PROPERTIES RUN_SERIAL TRUE)
endforeach()
