
#-----------------------------------------------------------------------------
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test MRMLCore vtkTeem)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ${CLP}Test
    --enumeration FractionalAnisotropy
    ${MRML_TEST_DATA}/helix-DTI.nhdr
    ${TEMP}/${CLP}Test.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

