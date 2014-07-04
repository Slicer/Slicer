
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP ConnectedComponent)

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/CTHeadAxialLabelTest.nrrd
            ${TEMP}/CTHeadAxialLabelTest.nrrd
  ModuleEntryPoint
   ${BASELINE}/CTHeadAxialLabel.nrrd
   ${TEMP}/CTHeadAxialLabelTest.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
