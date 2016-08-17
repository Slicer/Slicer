
#-----------------------------------------------------------------------------
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test
  ${CLP}Lib
  ${${MODULE_NAME}_TARGET_LIBRARIES}
  ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES}
  )
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  GrayscaleModelMakerTest
    --threshold 300
    --name CTFace
    --smooth 15
    --decimate 0.95
    --splitnormals
    --pointnormals
    ${TEST_DATA}/CTHeadAxial.nhdr
    ${TEMP}GrayscaleModelMakerTest.vtp
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
