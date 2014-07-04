
#-----------------------------------------------------------------------------
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
include_directories(
  ${vtkITK_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  ${vtkTeem_INCLUDE_DIRS}
  )
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  ${${MODULE_NAME}_TARGET_LIBRARIES}
  ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES}
  )
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ${testname}
    -m ${MRML_TEST_DATA}/helix-DWI-otsu-no_islands-05.nrrd
    ${MRML_TEST_DATA}/helix-DWI.nhdr
    ${TEMP}/${testname}_dti.nhdr
    ${TEMP}/${testname}_baseline.nhdr
    )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

