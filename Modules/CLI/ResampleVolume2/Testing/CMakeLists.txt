
#-----------------------------------------------------------------------------
# ResampleVolume2 tests
#-----------------------------------------------------------------------------
set (CLP ResampleVolume2)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(TRANSFORMS_DIR ${Slicer_SOURCE_DIR}/Modules/CLI/DiffusionApplications/ResampleDTI/Testing/TestData )

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test.nrrd
  ModuleEntryPoint
    --Reference ${TEST_DATA}/MRHeadResampled.nhdr
    --number_of_thread 0
    --transform_matrix 1,0,0,0,1,0,0,0,1,0,0,0
    --transform rt
    --interpolation linear
    --window_function c
    --spline_order 3
    --spacing 0,0,0
    --size 0,0,0
    --direction_matrix 0,0,0,0,0,0,0,0,0
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(TransformFile ${TRANSFORMS_DIR}/rotation.tfm)
set(testname ${CLP}RotationNNTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledRotationNN.nrrd
    ${TEMP}/ResampleVolume2Test2.nrrd
  ModuleEntryPoint
    -f ${TransformFile}
    --interpolation nn
    -c
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test2.nrrd
    -n 8
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(RotationAndAffineFile ${TRANSFORMS_DIR}/rotationAndAffine.tfm)
set(testname ${CLP}RotationAndAffineTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledRotationAndAffine.nrrd
    ${TEMP}/ResampleVolume2Test3.nrrd
  ModuleEntryPoint
    -f ${RotationAndAffineFile}
    --interpolation linear
    -c
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test3.nrrd
    --transform_order input-to-output
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(BSplineFile ${Slicer_SOURCE_DIR}/Testing/Data/Input/MRHeadResampledBSplineTransform.tfm)
set(testname ${CLP}BSplineWSInterpolationTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledBSplineWSInterpolationTest.nrrd
    ${TEMP}/ResampleVolume2Test4.nrrd
  ModuleEntryPoint
    -f ${BSplineFile}
    --interpolation ws
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test4.nrrd
    --transform_order input-to-output

  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(AffineFile ${TRANSFORMS_DIR}/affine.tfm)
set(testname ${CLP}BSplineInterpolationTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledBSplineInterpolationTest.nrrd
    ${TEMP}/ResampleVolume2Test5.nrrd
  ModuleEntryPoint
    -f ${AffineFile}
    --interpolation bs
    -o 3
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test5.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(HFieldFile ${Slicer_SOURCE_DIR}/Testing/Data/Input/MRHeadResampledHField.nrrd)
set(testname ${CLP}HFieldTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledHFieldTest.nrrd
    ${TEMP}/ResampleVolume2Test6.nrrd
  ModuleEntryPoint
    -H ${HFieldFile}
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/ResampleVolume2Test6.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

