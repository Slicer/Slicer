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

add_executable(DetectGrowth DetectGrowth.cxx)
#slicer3_set_plugins_output_path(DetectGrowth)
target_link_libraries(DetectGrowth ITKCommon ITKIO)
#slicer3_install_plugins(DetectGrowth) 
install(TARGETS DetectGrowth RUNTIME 
        DESTINATION bin COMPONENT Development
)

add_executable(applyDeformationITK applyDeformationITK.cxx)
#slicer3_set_plugins_output_path(applyDeformationITK)
target_link_libraries(applyDeformationITK ITKNumerics ITKIO)
#slicer3_install_plugins(applyDeformationITK)
install(TARGETS applyDeformationITK RUNTIME 
        DESTINATION bin COMPONENT Development
)


add_executable(DetectGrowthSegmentation DetectGrowthSegmentation.cxx)
#slicer3_set_plugins_output_path(DetectGrowthSegmentation)
target_link_libraries(DetectGrowthSegmentation ITKCommon ITKIO)
install(TARGETS DetectGrowthSegmentation RUNTIME 
        DESTINATION bin COMPONENT Development
)

#slicer3_install_plugins(DetectGrowthSegmentation)
