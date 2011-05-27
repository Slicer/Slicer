if(BRAINS_BUILD)
  set(TESTING_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else(BRAINS_BUILD)
  set(TESTING_BINARY_DIR ${BRAINSROIAuto_BINARY_DIR}/bin)
endif(BRAINS_BUILD)

add_executable(BRATestImage TestImage.cxx)
target_link_libraries(BRATestImage ${ITK_LIBRARIES})
set_target_properties(BRATestImage PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_BINARY_DIR})

add_executable(BRATestCompare BRATestCompare.cxx)
target_link_libraries(BRATestCompare ${ITK_LIBRARIES})
set_target_properties(BRATestCompare PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_BINARY_DIR})

include(${BRAINS_CMAKE_HELPER_DIR}/MIDAS.cmake)
set(MIDAS_KEY_DIR "${PROJECT_SOURCE_DIR}/../BRAINSCommonLib/TestData")

#add_test(
#  NAME BRATestImage 
#  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:BRATestImage>
#    ${BRAINSROIAuto_BINARY_DIR}/sphere.nii.gz 
#    ${BRAINSROIAuto_BINARY_DIR}/GeneratedMaskSphere.nii.gz
#  )

midas_add_test(
  NAME BRAINSROIAutoTest_GenerateBrainMask
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:BRAINSROIAuto>
  --inputVolume MIDAS{test.nii.gz.md5}
  --outputROIMaskVolume ${TESTING_BINARY_DIR}/BRAINSROIAutoTest_GenerateBrainMask.nii.gz
  --ROIAutoDilateSize 10 
  --outputVolumePixelType uchar
  )
set_tests_properties ( BRAINSROIAutoTest_GenerateBrainMask PROPERTIES DEPENDS BRATestImage)

midas_add_test(
  NAME BRAINSROIAutoTest_CompareBrainMasks
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:BRATestCompare>
  MIDAS{BRAINSROIAutoTest_GenerateBrainMask.result.nii.gz.md5}
  ${TESTING_BINARY_DIR}/BRAINSROIAutoTest_GenerateBrainMask.nii.gz
  )
set_tests_properties ( BRAINSROIAutoTest_CompareBrainMasks PROPERTIES DEPENDS RunBRAINSROIAuto)

