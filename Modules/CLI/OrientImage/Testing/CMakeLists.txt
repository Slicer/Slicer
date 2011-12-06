
#-----------------------------------------------------------------------------
# OrientImage tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP OrientImage)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname OrientImageTest1)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OrientImageTestAxial.nrrd
  ${TEMP}/OrientImageTestAxial.nrrd
  ModuleEntryPoint
  --orientation Axial ${MRML_TEST_DATA}/fixed.nrrd ${TEMP}/OrientImageTestAxial.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname OrientImageTest2)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OrientImageTestCoronal.nrrd
  ${TEMP}/OrientImageTestCoronal.nrrd
  ModuleEntryPoint
  --orientation Coronal ${MRML_TEST_DATA}/fixed.nrrd ${TEMP}/OrientImageTestCoronal.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname OrientImageTest3)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/OrientImageTestSagittal.nrrd
  ${TEMP}/OrientImageTestSagittal.nrrd
  ModuleEntryPoint
  --orientation Sagittal ${MRML_TEST_DATA}/fixed.nrrd ${TEMP}/OrientImageTestSagittal.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

