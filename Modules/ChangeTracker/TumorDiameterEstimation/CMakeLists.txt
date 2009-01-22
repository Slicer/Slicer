PROJECT(TumorDiameterEstimation)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)


include_directories(Code/LookAt)
add_executable(DiameterEstimator Code/DiameterEstimator.cxx)
#slicer3_set_plugins_output_path(${CLP})
target_link_libraries(DiameterEstimator ITKCommon ITKIO)
#slicer3_install_plugins(DiameterEstimator)
