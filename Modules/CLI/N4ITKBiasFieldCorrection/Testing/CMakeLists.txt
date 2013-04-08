
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/he3corrected.nii.gz ${TEMP}/he3corrected.nii.gz
  ModuleEntryPoint
  --inputimage ${TEST_DATA}/he3volume.nii.gz --maskimage ${TEST_DATA}/he3mask.nii.gz
  --outputimage ${TEMP}/he3corrected.nii.gz --outputbiasfield ${TEMP}/he3biasfield.nii.gz
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
