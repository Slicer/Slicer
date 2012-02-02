
set(Launcher_Command ${Slicer_LAUNCH_COMMAND})
if(USE_VALGRIND)
  set(Launcher_Command "${Slicer_BINARY_DIR}/Scripts/runValgrindForOneModule.sh")
endif()

set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
set(CLP ResampleDTIVolume)

set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
set(TransformedImage ${TEMP}/dt-helix-transformed.nrrd )

set(OriginalImage ${INPUT}/dt-helix.nrrd)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

#-----------------------------------------------------------------------------
set(TransformFile ${INPUT}/rotation.tfm )
set(TransformedImage1 ${TEMP}/dt-helix-transformed1.nrrd )
set(testname ${CLP}RotationNNTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${BASELINE}/dt-helix-ref-Rotated.nrrd
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
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${BASELINE}/dt-helix-ref-RotationAndAffine.nrrd
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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(BSplineFile ${INPUT}/FastNonrigidBSplineregistrationTransform.tfm)
set(TransformedImage3 ${TEMP}/dt-helix-transformed3.nrrd )
set(testname ${CLP}BSplineWSInterpolationTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${BASELINE}/dt-helix-ref-BS.nrrd
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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(AffineFile ${INPUT}/affine.tfm)
set(TransformedImage4 ${TEMP}/dt-helix-transformed4.nrrd )
set(testname ${CLP}BSplineInterpolationTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${BASELINE}/dt-helix-ref-BSInterpolation.nrrd
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
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(HFieldFile ${INPUT}/deformationField.nrrd)
set(TransformedImage5 ${TEMP}/dt-helix-transformed5.nrrd )
set(testname ${CLP}HFieldTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${BASELINE}/dt-helix-ref-HField.nrrd
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
message(WARNING "warning: Test named 'itkTestMainExtendedScalarTest' associated with ResampleDTI should be reviewed !")
# TODO The baseline associated with each command line module has been moved within
#      the Data/Baseline subdirectory of each module. For that reason, ResampleDTI shouldn;t
#      reference the baseline 'AddTest_DOUBLE.mha' as input data.
#Test itkTestMainExtended.h
#copy scalar file to binary directory so that if an error occurs the output files will be created in the binary directory
#set(SourceScalarFile ${Slicer_SOURCE_DIR}/Testing/Data/Baseline/CLI/AddTest_DOUBLE.mha )
#file( COPY ${SourceScalarFile} DESTINATION ${TEMP})
#set( BinaryScalarFile ${TEMP}/AddTest_DOUBLE.mha)

#set(testname itkTestMainExtendedScalarTest)
#add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
# --compare
#   ${BinaryScalarFile}
#   ${BinaryScalarFile}
#  ModuleEntryPoint
#    --interpolation nn
#    ${OriginalImage}
#    ${TransformedImage}
#  )
#set_property(TEST ${testname} PROPERTY LABELS ${CLP})

#-----------------------------------------------------------------------------
set(TransformedImage6 ${TEMP}/dt-helix-transformed6.nrrd )
set(testname itkTestMainExtendedFailedTest)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  --compare
    ${BASELINE}/dt-helix-ref-HField.nrrd
    ${TransformedImage6}
  --compareIntensityTolerance 0
  ModuleEntryPoint
    --interpolation nn
    ${OriginalImage}
    ${TransformedImage6}
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
set_tests_properties( itkTestMainExtendedFailedTest PROPERTIES WILL_FAIL true)
