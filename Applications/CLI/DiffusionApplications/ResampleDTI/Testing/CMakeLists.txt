
set(Launcher_Command ${Slicer_LAUNCH_COMMAND})
if(USE_VALGRIND)
  set(Launcher_Command "${Slicer_BINARY_DIR}/Scripts/runValgrindForOneModule.sh")
endif()

set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
set(TransformedImage ${TEMP}/dt-helix-transformed.nrrd )
set(SOURCE_DIR ${Slicer_SOURCE_DIR}/Applications/CLI/DiffusionApplications/ResampleDTI/Testing/TestData )
set(OriginalImage ${SOURCE_DIR}/dt-helix.nrrd )

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
SET_TARGET_PROPERTIES(${CLP}Test PROPERTIES LABELS ${CLP})

#-----------------------------------------------------------------------------
set(TransformFile ${SOURCE_DIR}/rotation.tfm )
set(TransformedImage1 ${TEMP}/dt-helix-transformed1.nrrd )
set(ReferenceImageRotationNN ${SOURCE_DIR}/dt-helix-ref-Rotated.nrrd)
set(testname ${CLP}RotationNNTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${ReferenceImageRotationNN}
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(TransformedImage2 ${TEMP}/dt-helix-transformed2.nrrd )
set(ReferenceImageRotationAndAffine ${SOURCE_DIR}/dt-helix-ref-RotationAndAffine.nrrd)
set(RotationAndAffineFile ${SOURCE_DIR}/rotationAndAffine.tfm )
set(testname ${CLP}2RigidTransformsLinearTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${ReferenceImageRotationAndAffine}
    ${TransformedImage2}
  --compareIntensityTolerance 1
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
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(TransformedImage3 ${TEMP}/dt-helix-transformed3.nrrd )
set(ReferenceImageBSplineWS ${SOURCE_DIR}/dt-helix-ref-BS.nrrd)
set(BSplineFile ${SOURCE_DIR}/FastNonrigidBSplineregistrationTransform.tfm)
set(testname ${CLP}BSplineWSInterpolationTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${ReferenceImageBSplineWS}
    ${TransformedImage3}
  --compareIntensityTolerance 1
  ModuleEntryPoint
    -f ${BSplineFile}
    --interpolation ws
    ${OriginalImage}
    ${TransformedImage3}
    --correction nearest
    -n 8
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(TransformedImage4 ${TEMP}/dt-helix-transformed4.nrrd )
set(ReferenceImageBSplineInterpolation ${SOURCE_DIR}/dt-helix-ref-BSInterpolation.nrrd)
set(AffineFile ${SOURCE_DIR}/affine.tfm)
set(testname ${CLP}BSplineInterpolationTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${ReferenceImageBSplineInterpolation}
    ${TransformedImage4}
  --compareIntensityTolerance 0
  ModuleEntryPoint
    -f ${AffineFile}
    --interpolation bs
    ${OriginalImage}
    ${TransformedImage4}
    --correction none
    -o 3
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(TransformedImage5 ${TEMP}/dt-helix-transformed5.nrrd )
set(ReferenceImageHField ${SOURCE_DIR}/dt-helix-ref-HField.nrrd)
set(HFieldFile ${SOURCE_DIR}/deformationField.nrrd)
set(testname ${CLP}HFieldTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${ReferenceImageHField}
    ${TransformedImage5}
  --compareIntensityTolerance 0
  ModuleEntryPoint
    -H ${HFieldFile}
    ${OriginalImage}
    ${TransformedImage5}
    -n 8
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})


#-----------------------------------------------------------------------------
#Test itkTestMainExtended.h
#copy scalar file to binary directory so that if an error occurs the output files will be created in the binary directory
set(SourceScalarFile ${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI/AddTest_DOUBLE.mha )
file( COPY ${SourceScalarFile} DESTINATION ${TEMP} )
set( BinaryScalarFile ${TEMP}/AddTest_DOUBLE.mha )

set(testname itkTestMainExtendedScalarTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
 --compare
   ${BinaryScalarFile}
   ${BinaryScalarFile}
  ModuleEntryPoint
    --interpolation nn
    ${OriginalImage}
    ${TransformedImage}
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(TransformedImage6 ${TEMP}/dt-helix-transformed6.nrrd )
set(testname itkTestMainExtendedFailedTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${ReferenceImageHField}
    ${TransformedImage6}
  --compareIntensityTolerance 0
  ModuleEntryPoint
    --interpolation nn
    ${OriginalImage}
    ${TransformedImage6}
  )
SET_PROPERTY(TEST ${testname} PROPERTY LABELS ${CLP})
set_tests_properties( itkTestMainExtendedFailedTest PROPERTIES WILL_FAIL true)
