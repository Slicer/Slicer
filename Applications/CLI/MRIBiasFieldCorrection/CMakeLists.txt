#-----------------------------------------------------------------------------
#
#  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
#  Reserved.
#
#  See Doc/copyright/copyright.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Program:   3D Slicer
#  Module:    $RCSfile: CMakeLists.txt,v $
#  Date:      $Date: 2009/06/01 17:56:51 $
#  Version:   $Revision: 1.0 $
#  Author:    $Sylvain Jaume (MIT)$
#  URL:       $http://www.slicer.org/slicerWiki/index.php/Modules:MRIBiasFieldCorrection-Documentation-3.6$
#
#-----------------------------------------------------------------------------

project(MRIBiasFieldCorrection)

#-----------------------------------------------------------------------------
# Disable MSVC 8 warnings

if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif (WIN32)

#-----------------------------------------------------------------------------
# Build CLI library and executable

slicerMacroBuildCLI(
  NAME ${PROJECT_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/MIT_NAMIC_Logo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES} vtkIO vtkImaging
  )

#-----------------------------------------------------------------------------
# Testing

set(TEST_DATA_DIR ${PROJECT_SOURCE_DIR}/TestData)
set(BASELINE_DIR ${PROJECT_SOURCE_DIR}/Baseline)
set(OUTPUT_DIR ${Slicer_BINARY_DIR}/Testing)

if (BUILD_TESTING)

  #add_executable(ImageCompare ImageCompare.cxx)
  #target_link_libraries(ImageCompare
  #  ${ITK_LIBRARIES}
  #  )
  #set_target_properties(ImageCompare PROPERTIES LABELS ${CLP})

  set(testname ${PROJECT_NAME}_N3_Brain2D_1)
  add_test(NAME ${testname} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${PROJECT_NAME}>
    --inputImage
    ${TEST_DATA_DIR}/t81slice.nii.gz
    --inputMask
    ${TEST_DATA_DIR}/t81mask.nii.gz
    --outputImage
    ${OUTPUT_DIR}/t81corrected.nii.gz
    #--outputBiasField
    #${OUTPUT_DIR}/t81biasfield.nii.gz
    --algorithmType N3
    --shrinkFactor 2
    --maximumNumberOfIterations 50
    --numberOfFittingLevels 4)
  set_property(TEST ${testname} PROPERTY LABELS ${PROJECT_NAME})

  #set(testname ${PROJECT_NAME}_N3_Brain2D_CompareImage)
  #add_test(NAME ${testname} COMMAND $<TARGET_FILE:ImageCompare>
  #  ${OUTPUT_DIR}/t81corrected.nii.gz
  #  ${BASELINE_DIR}/t81corrected.nii.gz)
  #set_property(TEST ${testname} PROPERTY LABELS ${PROJECT_NAME})

  #set(testname ${PROJECT_NAME}_N3_Brain2D_CompareField)
  #add_test(NAME ${testname} COMMAND $<TARGET_FILE:ImageCompare>
  #  ${OUTPUT_DIR}/t81biasfield.nii.gz
  #  ${BASELINE_DIR}/t81biasfield.nii.gz)
  #set_property(TEST ${testname} PROPERTY LABELS ${PROJECT_NAME})

  set(testname ${PROJECT_NAME}_N3_Brain2D_2)
  add_test(NAME ${testname} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${PROJECT_NAME}>
    --inputImage
    ${TEST_DATA_DIR}/he3volume.nii.gz
    --inputMask
    ${TEST_DATA_DIR}/Input/he3mask.nii.gz
    --outputImage
    ${OUTPUT_DIR}/he3corrected.nii.gz
    #--outputBiasField
    #${OUTPUT_DIR}/he3biasfield.nii.gz
    --algorithmType N3
    --shrinkFactor 4
    --maximumNumberOfIterations 50)
  set_property(TEST ${testname} PROPERTY LABELS ${PROJECT_NAME})

  #set(testname ${PROJECT_NAME}_N3_Lungs3D_CompareImage)
  #add_test(NAME ${testname} COMMAND $<TARGET_FILE:ImageCompare>
  #  ${OUTPUT_DIR}/he3corrected.nii.gz
  #  ${BASELINE_DIR}/he3corrected.nii.gz)
  #set_property(TEST ${testname} PROPERTY LABELS ${PROJECT_NAME})

  #set(testname ${PROJECT_NAME}_N3_Lungs3D_CompareField)
  #add_test(NAME ${testname} COMMAND $<TARGET_FILE:ImageCompare>
  #  ${OUTPUT_DIR}/he3biasfield.nii.gz
  #  ${BASELINE_DIR}/he3biasfield.nii.gz)
  #set_property(TEST ${testname} PROPERTY LABELS ${PROJECT_NAME})

  set(testname ${PROJECT_NAME}_N4_Brain2D)
  add_test(NAME ${testname} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${PROJECT_NAME}>
    --inputImage
    ${TEST_DATA_DIR}/t81slice.nii.gz
    --inputMask
    ${TEST_DATA_DIR}/t81mask.nii.gz
    --outputImage
    ${OUTPUT_DIR}/t81corrected_n4.nii.gz
    #--outputBiasField
    #${OUTPUT_DIR}/t81biasfield_n4.nii.gz
    --algorithmType N4
    --shrinkFactor 2
    --numberOfFittingLevels 3
    --maximumNumberOfIterations 50
    --convergenceThreshold 0.001)
  set_property(TEST ${testname} PROPERTY LABELS ${PROJECT_NAME})

endif()
