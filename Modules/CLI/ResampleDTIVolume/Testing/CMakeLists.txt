
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

set(CLP ${MODULE_NAME})

if(NOT DEFINED SEM_DATA_MANAGEMENT_TARGET)
  set(SEM_DATA_MANAGEMENT_TARGET ${CLP}Data)
endif()

#-----------------------------------------------------------------------------
ctk_add_executable_utf8(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

#-----------------------------------------------------------------------------
set(OriginalImage DATA{${INPUT}/dt-helix.nrrd})

#-----------------------------------------------------------------------------
set(TransformFile ${INPUT}/rotation.tfm )
set(TransformedImage1 ${TEMP}/dt-helix-transformed1.nrrd )
set(testname ${CLP}RotationNNTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${BASELINE}/dt-helix-ref-Rotated.nrrd}
    ${TransformedImage1}
  --compareIntensityTolerance 0
  ModuleEntryPoint
    -f ${TransformFile}
    --interpolation nn
    -c
    ${OriginalImage}
    ${TransformedImage1}
    -n 8
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(RotationAndAffineFile ${INPUT}/rotationAndAffine.tfm )
set(TransformedImage2 ${TEMP}/dt-helix-transformed2.nrrd )
set(testname ${CLP}2RigidTransformsLinearTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${BASELINE}/dt-helix-ref-RotationAndAffine.nrrd}
    ${TransformedImage2}
  --compareIntensityTolerance 1e-10
  ModuleEntryPoint
    -f ${RotationAndAffineFile}
    -T FS
    --interpolation linear
    -c
    ${OriginalImage}
    ${TransformedImage2}
    --correction abs
    -n 8
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(BSplineFile ${INPUT}/FastNonrigidBSplineregistrationTransform.tfm)
set(TransformedImage3 ${TEMP}/dt-helix-transformed3.nrrd )
set(testname ${CLP}BSplineWSInterpolationTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${BASELINE}/dt-helix-ref-BS.nrrd}
    ${TransformedImage3}
  --compareIntensityTolerance 1e-10
  ModuleEntryPoint
    -f ${BSplineFile}
    --interpolation ws
    ${OriginalImage}
    ${TransformedImage3}
    --correction nearest
    -n 8
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(AffineFile ${INPUT}/affine.tfm)
set(TransformedImage4 ${TEMP}/dt-helix-transformed4.nrrd )
set(testname ${CLP}BSplineInterpolationTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${BASELINE}/dt-helix-ref-BSInterpolation.nrrd}
    ${TransformedImage4}
  --compareIntensityTolerance 1e-10
  ModuleEntryPoint
    -f ${AffineFile}
    --interpolation bs
    ${OriginalImage}
    ${TransformedImage4}
    --correction none
    -o 3
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(HFieldFile DATA{${INPUT}/deformationField.nrrd})
set(TransformedImage5 ${TEMP}/dt-helix-transformed5.nrrd )
set(testname ${CLP}HFieldTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${BASELINE}/dt-helix-ref-HField.nrrd}
    ${TransformedImage5}
  --compareIntensityTolerance 0
  ModuleEntryPoint
    -H ${HFieldFile}
    ${OriginalImage}
    ${TransformedImage5}
    -n 8
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})


#-----------------------------------------------------------------------------
#Test itkTestMainExtended.h
#copy scalar file to binary directory so that if an error occurs the output files will be created in the binary directory
#We run ResampleDTIVolume and do not care about the result. What interests us in that test is to make sure that by extending
#itkTestMainExtended.h to work with DTI, we did not break it and that it still works with scalar images too.
#Therefore, we compare a scalar image with itself and verify that the test passes successfully.
set(testname itkTestMainExtendedScalarTest)
set(TransformedImage7 ${TEMP}/dt-helix-transformed7.nrrd )
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
 --compare
   DATA{${BASELINE}/Brain_slice.nrrd}
   DATA{${BASELINE}/Brain_slice.nrrd}
  ModuleEntryPoint
    --interpolation nn
    ${OriginalImage}
    ${TransformedImage7}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(TransformedImage6 ${TEMP}/dt-helix-transformed6.nrrd )
set(testname itkTestMainExtendedFailedTest)
ExternalData_add_test(${SEM_DATA_MANAGEMENT_TARGET}
  NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  --compare
    DATA{${BASELINE}/dt-helix-ref-HField.nrrd}
    ${TransformedImage6}
  --compareIntensityTolerance 0
  ModuleEntryPoint
    --interpolation nn
    ${OriginalImage}
    ${TransformedImage6}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
set_tests_properties( itkTestMainExtendedFailedTest PROPERTIES WILL_FAIL true)

#-----------------------------------------------------------------------------
if(${SEM_DATA_MANAGEMENT_TARGET} STREQUAL ${CLP}Data)
  ExternalData_add_target(${CLP}Data)
endif()
