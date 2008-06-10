project(DetectingTumorGrowth)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

find_package(ITK)
if(ITK_FOUND)
    INCLUDE(${ITK_USE_FILE})
else(ITK_FOUND)
    MESSAGE(FATAL_ERROR
        "ITK not found. Please set ITK_DIR.")
endif(ITK_FOUND)

add_executable(DetectGrowth DetectGrowth.cxx)
slicer3_set_plugins_output_path(DetectGrowth)
target_link_libraries(DetectGrowth ITKCommon ITKIO)
slicer3_install_plugins(DetectGrowth)

add_executable(applyDeformationITK applyDeformationITK.cxx)
slicer3_set_plugins_output_path(applyDeformationITK)
target_link_libraries(applyDeformationITK ITKNumerics ITKIO)
slicer3_install_plugins(applyDeformationITK)

add_executable(DetectGrowthSegmentation DetectGrowthSegmentation.cxx)
slicer3_set_plugins_output_path(DetectGrowthSegmentation)
target_link_libraries(DetectGrowthSegmentation ITKCommon ITKIO)
slicer3_install_plugins(DetectGrowthSegmentation)



                           
