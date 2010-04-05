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
    --number_of_thread 0
    --transform_matrix 1,0,0,0,1,0,0,0,1,0,0,0
    --transform rt
    --space LPS
    --interpolation nn
    --window_function c
    --spline_order 3
    --spacing 0,0,0
    --size 0,0,0
    --direction_matrix 0,0,0,0,0,0,0,0,0
    ${MRML_TEST_DATA}/helix-DTI.nhdr
    ${TEMP}/ResampleDTITest.nhdr
  )

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
