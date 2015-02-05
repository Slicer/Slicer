
#-----------------------------------------------------------------------------
set(ResampleDTIVolume_INPUT ${ResampleDTIVolume_SOURCE_DIR}/Data/Input)
set(CLP ${MODULE_NAME})

#-----------------------------------------------------------------------------
add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/${testname}.nrrd
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
    ${TEMP}/${testname}.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(TransformFile ${ResampleDTIVolume_INPUT}/rotation.tfm)
set(testname ${CLP}RotationNNTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledRotationNN.nrrd
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -f ${TransformFile}
    --interpolation nn
    -c
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/${testname}.nrrd
    -n 8
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(RotationAndAffineFile ${ResampleDTIVolume_INPUT}/rotationAndAffine.tfm)
set(testname ${CLP}RotationAndAffineTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledRotationAndAffine.nrrd
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -f ${RotationAndAffineFile}
    --interpolation linear
    -c
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/${testname}.nrrd
    --transform_order input-to-output
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(BSplineFile ${Slicer_SOURCE_DIR}/Testing/Data/Input/MRHeadResampledBSplineTransform.tfm)
set(testname ${CLP}BSplineWSInterpolationTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledBSplineWSInterpolationTest.nrrd
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -f ${BSplineFile}
    --interpolation ws
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/${testname}.nrrd
    --transform_order input-to-output

  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(AffineFile ${ResampleDTIVolume_INPUT}/affine.tfm)
set(testname ${CLP}BSplineInterpolationTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledBSplineInterpolationTest.nrrd
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -f ${AffineFile}
    --interpolation bs
    -o 3
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/${testname}.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(HFieldFile ${Slicer_SOURCE_DIR}/Testing/Data/Input/MRHeadResampledHField.nrrd)
set(testname ${CLP}HFieldTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    ${TEST_DATA}/MRHeadResampledHFieldTest.nrrd
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -H ${HFieldFile}
    ${TEST_DATA}/MRHeadResampled.nhdr
    ${TEMP}/${testname}.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

