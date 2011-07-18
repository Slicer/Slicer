
#-----------------------------------------------------------------------------
# ImageReadDicomWrite tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP ImageReadDicomWrite)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare ${BASELINE}/ImageReadDicomWriteTest.dcm
            ${TEMP}/CTHeadAxialDicom0040.dcm
  ModuleEntryPoint
    --patientName Austrialian
    --patientID 8775070
    --patientComments "A volunteer"
    --studyID 123456
    --studyDate 20090102
    --studyComments Resampled
    --studyDescription None
    --modality CT
    --manufacturer "GE Medical Systems"
    --model CT8800
    --seriesNumber 123456
    --seriesDescription None
    --dicomDirectory ${TEMP}
    --dicomPrefix CTHeadAxialDicom
    ${TEST_DATA}/CTHeadAxial.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

