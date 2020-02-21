
#-----------------------------------------------------------------------------
set(ResampleDTIVolume_INPUT ${ResampleDTIVolume_SOURCE_DIR}/Data/Input)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(CLP ${MODULE_NAME})

if(NOT DEFINED SEM_DATA_MANAGEMENT_TARGET)
  set(SEM_DATA_MANAGEMENT_TARGET ${CLP}Data)
endif()

#-----------------------------------------------------------------------------
ctk_add_executable_utf8(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}Test)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    --Reference DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    --number_of_thread 0
    --transform_matrix 1,0,0,0,1,0,0,0,1,0,0,0
    --transform rt
    --interpolation linear
    --window_function c
    --spline_order 3
    --spacing 0,0,0
    --size 0,0,0
    --direction_matrix 0,0,0,0,0,0,0,0,0
    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    ${TEMP}/${testname}.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(TransformFile ${ResampleDTIVolume_INPUT}/rotation.tfm)
set(testname ${CLP}RotationNNTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${INPUT}/MRHeadResampledRotationNN.nrrd}
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -f ${TransformFile}
    --interpolation nn
    -c
    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    ${TEMP}/${testname}.nrrd
    -n 8
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(RotationAndAffineFile ${ResampleDTIVolume_INPUT}/rotationAndAffine.tfm)
set(testname ${CLP}RotationAndAffineTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${INPUT}/MRHeadResampledRotationAndAffine.nrrd}
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -f ${RotationAndAffineFile}
    --interpolation linear
    -c
    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    ${TEMP}/${testname}.nrrd
    --transform_order input-to-output
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(BSplineFile ${Slicer_SOURCE_DIR}/Testing/Data/Input/MRHeadResampledBSplineTransform.tfm)
set(testname ${CLP}BSplineWSInterpolationTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${INPUT}/MRHeadResampledBSplineWSInterpolationTest.nrrd}
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -f ${BSplineFile}
    --interpolation ws
    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    ${TEMP}/${testname}.nrrd
    --transform_order input-to-output

  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(AffineFile ${ResampleDTIVolume_INPUT}/affine.tfm)
set(testname ${CLP}BSplineInterpolationTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${INPUT}/MRHeadResampledBSplineInterpolationTest.nrrd}
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -f ${AffineFile}
    --interpolation bs
    -o 3
    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    ${TEMP}/${testname}.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}HFieldTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${INPUT}/MRHeadResampledHFieldTest.nrrd}
    ${TEMP}/${testname}.nrrd
  ModuleEntryPoint
    -H DATA{${INPUT}/MRHeadResampledHField.nrrd}
    DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
    ${TEMP}/${testname}.nrrd
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
