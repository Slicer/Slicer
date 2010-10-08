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

  option(DISABLE_MSVC8_DEPRECATED_WARNINGS
    "Disable Visual Studio 8 deprecated warnings" ON)
  mark_as_advanced(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)

  if (DISABLE_MSVC8_DEPRECATED_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif (DISABLE_MSVC8_DEPRECATED_WARNINGS)

endif (WIN32)


#-----------------------------------------------------------------------------
# Source files

set(${PROJECT_NAME}_SOURCE ${PROJECT_NAME}.cxx)

generateclp(${PROJECT_NAME}_SOURCE ${PROJECT_NAME}.xml
  ${Slicer_SOURCE_DIR}/Resources/MIT_NAMIC_Logo.h)


#-----------------------------------------------------------------------------
# Build library

add_library(MRIBiasFieldCorrectionModule SHARED
  ${${PROJECT_NAME}_SOURCE})

slicer3_set_plugins_output_path(${PROJECT_NAME}Module)

set_target_properties(${PROJECT_NAME}Module PROPERTIES
  COMPILE_FLAGS "-Dmain=ModuleEntryPoint")

target_link_libraries(${PROJECT_NAME}Module
  ITKIO
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  vtkIO
  vtkImaging)


#-----------------------------------------------------------------------------
# Build executable

add_executable(${PROJECT_NAME}
  ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)

slicer3_set_plugins_output_path(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}Module)

add_executable(ImageCompare ImageCompare.cxx) 

target_link_libraries(ImageCompare
  ITKIO
  ITKNumerics
  ITKStatistics)

#-----------------------------------------------------------------------------
# Install

if (Slicer_SOURCE_DIR)

  set(TARGETS ${PROJECT_NAME}Module ${PROJECT_NAME})

  slicer3_install_plugins(${TARGETS})

endif (Slicer_SOURCE_DIR)


#-----------------------------------------------------------------------------
# Testing

set(Slicer_EXE ${Slicer_BINARY_DIR}/Slicer3)

set(TEST_DATA_DIR ${PROJECT_SOURCE_DIR}/TestData)
set(BASELINE_DIR ${PROJECT_SOURCE_DIR}/Baseline)
set(OUTPUT_DIR ${Slicer_BINARY_DIR}/Testing)

if (BUILD_TESTING)

  add_test(${PROJECT_NAME}_N3_Brain2D
    ${Slicer_EXE}
    --launch
    ${PROJECT_NAME}
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

  #add_test(${PROJECT_NAME}_N3_Brain2D_CompareImage
  #  ImageCompare
  #  ${OUTPUT_DIR}/t81corrected.nii.gz 
  #  ${BASELINE_DIR}/t81corrected.nii.gz)

  #add_test(${PROJECT_NAME}_N3_Brain2D_CompareField
  #  ImageCompare
  #  ${OUTPUT_DIR}/t81biasfield.nii.gz 
  #  ${BASELINE_DIR}/t81biasfield.nii.gz)

  add_test(${PROJECT_NAME}_N3_Lungs3D
    ${Slicer_EXE}
    --launch
    ${PROJECT_NAME}
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

  #add_test(${PROJECT_NAME}_N3_Lungs3D_CompareImage
  #  ImageCompare
  #  ${OUTPUT_DIR}/he3corrected.nii.gz 
  #  ${BASELINE_DIR}/he3corrected.nii.gz)

  #add_test(${PROJECT_NAME}_N3_Lungs3D_CompareField
  #  ImageCompare
  #  ${OUTPUT_DIR}/he3biasfield.nii.gz 
  #  ${BASELINE_DIR}/he3biasfield.nii.gz)

  add_test(${PROJECT_NAME}_N4_Brain2D
    ${Slicer_EXE}
    --launch
    ${PROJECT_NAME}
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

endif (BUILD_TESTING)
