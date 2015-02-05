
#-----------------------------------------------------------------------------
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --integer 30
    --double 30
    -f 1.3,2,-14
    --string_vector foo,bar,foobar
    --enumeration Bill
    --boolean1
    --transform1 ${TEST_DATA}/ExecutionModelTourTest.mrml\#vtkMRMLLinearTransformNode1
    --transform2 ${TEST_DATA}/ExecutionModelTourTest.mrml\#vtkMRMLLinearTransformNode2
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEST_DATA}/CTHeadAxial.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
