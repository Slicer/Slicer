
#-----------------------------------------------------------------------------
# Mask tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP Mask)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/MaskedVolume.nrrd
  ${TEMP}/MaskedVolume.nrrd
  ModuleEntryPoint --label 1
  ${TEST_DATA}/CTHeadAxial.nhdr ${TEST_DATA}/CTHeadAxialMask.nrrd ${TEMP}/MaskedVolume.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

