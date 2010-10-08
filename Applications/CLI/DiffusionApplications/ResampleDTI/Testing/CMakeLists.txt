#
# Test executable
#


include( CTest )




# ResampleDTI tests
set(Slicer_EXE ${Slicer_BINARY_DIR}/Slicer3 )

set(ValgrindScript "${Slicer_BINARY_DIR}/Scripts/runValgrindForOneModule.sh")

if(USE_VALGRIND)
  set(Launcher_Command ${ValgrindScript})
  set(TestLocation "${Slicer_BINARY_DIR}/bin/")
else(USE_VALGRIND)
  set(Launcher_Command ${Slicer_EXE} --launch)
  set(TestLocation "")
endif(USE_VALGRIND)

set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
set(ResampleTest ${CLP}Test)
set(TransformedImage ${TEMP}/dt-helix-transformed.nrrd )
set(SOURCE_DIR ${Slicer_SOURCE_DIR}/Applications/CLI/DiffusionApplications/ResampleDTI/Testing/TestData )
set(OriginalImage ${SOURCE_DIR}/dt-helix.nrrd )

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)

set(TransformFile ${SOURCE_DIR}/rotation.tfm )
set(TransformedImage1 ${TEMP}/dt-helix-transformed1.nrrd )
set(ReferenceImageRotationNN ${SOURCE_DIR}/dt-helix-ref-Rotated.nrrd)
add_test(${CLP}RotationNNTest ${Launcher_Command} ${TestLocation}${ResampleTest}
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

set(TransformedImage2 ${TEMP}/dt-helix-transformed2.nrrd )
set(ReferenceImageRotationAndAffine ${SOURCE_DIR}/dt-helix-ref-RotationAndAffine.nrrd)
set(RotationAndAffineFile ${SOURCE_DIR}/rotationAndAffine.tfm )
add_test(${CLP}2RigidTransformsLinearTest ${Launcher_Command} ${TestLocation}${ResampleTest}
  --compare
    ${ReferenceImageRotationAndAffine}
    ${TransformedImage2}
  --compareIntensityTolerance 0
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

set(TransformedImage3 ${TEMP}/dt-helix-transformed3.nrrd )
set(ReferenceImageBSplineWS ${SOURCE_DIR}/dt-helix-ref-BS.nrrd)
set(BSplineFile ${SOURCE_DIR}/FastNonrigidBSplineregistrationTransform.tfm)
add_test(${CLP}BSplineWSInterpolationTest ${Launcher_Command} ${TestLocation}${ResampleTest}
  --compare
    ${ReferenceImageBSplineWS}
    ${TransformedImage3}
  --compareIntensityTolerance 0
  ModuleEntryPoint
    -f ${BSplineFile}
    --interpolation ws
    ${OriginalImage}
    ${TransformedImage3}
    --correction nearest
    -n 8
  )

set(TransformedImage4 ${TEMP}/dt-helix-transformed4.nrrd )
set(ReferenceImageBSplineInterpolation ${SOURCE_DIR}/dt-helix-ref-BSInterpolation.nrrd)
set(AffineFile ${SOURCE_DIR}/affine.tfm)
add_test(${CLP}BSplineInterpolationTest ${Launcher_Command} ${TestLocation}${ResampleTest}
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

set(TransformedImage5 ${TEMP}/dt-helix-transformed5.nrrd )
set(ReferenceImageHField ${SOURCE_DIR}/dt-helix-ref-HField.nrrd)
set(HFieldFile ${SOURCE_DIR}/deformationField.nrrd)
add_test(${CLP}HFieldTest ${Launcher_Command} ${TestLocation}${ResampleTest}
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





#Test itkTestMainExtended.h
#copy scalar file to binary directory so that if an error occurs the output files will be created in the binary directory
set(SourceScalarFile ${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI/AddTest_DOUBLE.mha )
file( COPY ${SourceScalarFile} DESTINATION ${TEMP} )
set( BinaryScalarFile ${TEMP}/AddTest_DOUBLE.mha )

add_test( itkTestMainExtendedScalarTest ${Launcher_Command} ${TestLocation}${ResampleTest}
 --compare
   ${BinaryScalarFile}
   ${BinaryScalarFile}
  ModuleEntryPoint
    --interpolation nn
    ${OriginalImage}
    ${TransformedImage}
  )

set(TransformedImage6 ${TEMP}/dt-helix-transformed6.nrrd )
add_test(itkTestMainExtendedFailedTest ${Launcher_Command} ${TestLocation}${ResampleTest}
  --compare
    ${ReferenceImageHField}
    ${TransformedImage6}
  --compareIntensityTolerance 0
  ModuleEntryPoint
    --interpolation nn
    ${OriginalImage}
    ${TransformedImage6}
  )
set_tests_properties( itkTestMainExtendedFailedTest PROPERTIES WILL_FAIL true)
