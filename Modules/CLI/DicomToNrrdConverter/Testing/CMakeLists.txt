

set(CLP DicomToNrrdConverter)
add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(TEST_DATA "${Slicer_SOURCE_DIR}/Testing/Data/Input")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

# DicomToNrrdConverter tests
add_test(
    NAME ${CLP}Test
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
    ModuleEntryPoint
    --useBMatrixGradientDirections
    --inputDicomDirectory ${TEST_DATA}/DWIDicom
    --outputDirectory ${TEMP}
    --outputVolume DWIDicom.nhdr
  )
set_property(TEST ${CLP}Test PROPERTY LABELS ${CLP})

