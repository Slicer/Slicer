#
# Test executable
#

##################################
#
# Slicer3 is needed to resolve shared libraries
#
set(Slicer3_EXE ${Slicer3_BINARY_DIR}/Slicer3 )
set(MRML_TEST_DATA "${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/TestData")
set(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")

# ResampleDTI tests

set (CLP ResampleDTI)
add_test(${CLP}Test ${Slicer3_EXE} --launch ${CLP}Test
  ModuleEntryPoint
    --Reference ${MRML_TEST_DATA}/helix-DTI.nhdr
    --interpolation nn
    ${MRML_TEST_DATA}/helix-DTI.nhdr
    ${TEMP}/ResampleDTITest.nhdr
  )

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
