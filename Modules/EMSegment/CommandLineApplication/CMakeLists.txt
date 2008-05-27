project(EMSegmentCommandLineExecutable)

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
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${EMSegment_SOURCE_DIR}/CommandLineApplication
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Sources

set(CLP_MODULE EMSegmentCommandLine)
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
  EMSegment
  vtkCommon
  )

slicer3_install_plugins(${CLP_MODULE})

# --------------------------------------------------------------------------
# CLGUI

set(EMSEG_GENERATE_CLGUI 1)

if(EMSEG_GENERATE_CLGUI)
  set(CLP_MODULE_GUI EMSegmentCommandLine_GUIVersion)

  set(${CLP_MODULE_GUI}_SOURCE
    ${CLP_MODULE_GUI}.cxx
    )

  generateclp(
    ${CLP_MODULE_GUI}_SOURCE 
    ${CLP_MODULE_GUI}.xml
    )

  add_executable(${CLP_MODULE_GUI} ${${CLP_MODULE_GUI}_SOURCE})
  slicer3_set_plugins_output_path(${CLP_MODULE_GUI})
  set_target_properties(${CLP_MODULE_GUI}
    PROPERTIES COMPILE_FLAGS "-DEM_CL_GUI"
    )
  target_link_libraries(
    ${CLP_MODULE_GUI}
    EMSegment
    vtkCommon
    )

  slicer3_install_plugins(${CLP_MODULE_GUI})

  add_library(${CLP_MODULE_GUI}Lib SHARED ${${CLP_MODULE_GUI}_SOURCE})
  slicer3_set_plugins_output_path(${CLP_MODULE_GUI}Lib)
  set_target_properties(${CLP_MODULE_GUI}Lib 
    PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DEM_CL_GUI"
    )
  target_link_libraries(
    ${CLP_MODULE_GUI}Lib
    EMSegment
    vtkCommon
    )
  #slicer3_install_plugins(${CLP_MODULE_GUI}Lib)
endif(EMSEG_GENERATE_CLGUI)
