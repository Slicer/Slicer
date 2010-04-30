if(BRAINS_BUILD)
  set(TESTING_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else(BRAINS_BUILD)
  set(TESTING_BINARY_DIR ${BRAINSROIAuto_BINARY_DIR}/bin)
endif(BRAINS_BUILD)

add_executable(BRATestImage TestImage.cxx)
target_link_libraries(BRATestImage ${ITK_LIBRARIES})
set_target_properties(BRATestImage PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSROIAuto_BINARY_DIR})

add_executable(BRATestCompare BRATestCompare.cxx)
target_link_libraries(BRATestCompare ${ITK_LIBRARIES})
set_target_properties(BRATestCompare PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BRAINSROIAuto_BINARY_DIR})

add_test(BRATestImage BRATestImage ${BRAINSROIAuto_BINARY_DIR}/sphere.nii.gz ${BRAINSROIAuto_BINARY_DIR}/GeneratedMaskSphere.nii.gz)

add_test(RunBRAINSROIAuto ${TESTING_BINARY_DIR}/BRAINSROIAuto
  --inputVolume ${BRAINSROIAuto_BINARY_DIR}/sphere.nii.gz
  --outputClippedVolumeROI ${BRAINSROIAuto_BINARY_DIR}/clippedSphere.nii.gz
  --outputROIMaskVolume ${BRAINSROIAuto_BINARY_DIR}/maskSphere.nii.gz)
set_tests_properties ( RunBRAINSROIAuto PROPERTIES DEPENDS BRATestImage)

add_test(CompareMasks BRATestCompare
  ${BRAINSROIAuto_BINARY_DIR}/GeneratedMaskSphere.nii.gz
  ${BRAINSROIAuto_BINARY_DIR}/maskSphere.nii.gz)
set_tests_properties ( CompareMasks PROPERTIES DEPENDS RunBRAINSROIAuto)

