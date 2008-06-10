project(TumorGrowthCommandLineExecutable)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${TumorGrowth_SOURCE_DIR}
  ${TumorGrowth_SOURCE_DIR}/CommandLineApplication
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${vtkTeem_SOURCE_DIR} 
  ${vtkTeem_BINARY_DIR}
  )

# --------------------------------------------------------------------------
# Sources

set(CLP_MODULE TumorGrowthCommandLine)
set(${CLP_MODULE}_SOURCE
  ${CLP_MODULE}.cxx
  )
generateclp(
  ${CLP_MODULE}_SOURCE 
  ${CLP_MODULE}.xml)

# --------------------------------------------------------------------------
# Build and install the exe

add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE})
slicer3_set_plugins_output_path(${CLP_MODULE})
target_link_libraries(
  ${CLP_MODULE}
  TumorGrowth
  ITKIO 
  ITKCommon 
  vtkTeem
  )

slicer3_install_plugins(${CLP_MODULE})
slicer3_set_plugins_output_path(${CLP})
slicer3_install_plugins(${CLP})


