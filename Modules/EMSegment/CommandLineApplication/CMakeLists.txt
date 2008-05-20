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
  slicer3_set_default_modules_install_prefix()
endif(NOT Slicer3_SOURCE_DIR)

slicer3_set_default_plugins_output_path()

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
target_link_libraries(
  ${CLP_MODULE}
  EMSegment
  vtkCommon
  )

install(TARGETS ${CLP_MODULE}
  RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
  )

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
  set_target_properties(${CLP_MODULE_GUI}
    PROPERTIES COMPILE_FLAGS "-DEM_CL_GUI"
    )
  target_link_libraries(
    ${CLP_MODULE_GUI}
    EMSegment
    vtkCommon
    )

  install(TARGETS ${CLP_MODULE_GUI}
    RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
    )

  add_library(${CLP_MODULE_GUI}Lib SHARED ${${CLP_MODULE_GUI}_SOURCE})
  set_target_properties(${CLP_MODULE_GUI}Lib 
    PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DEM_CL_GUI"
    )
  target_link_libraries(
    ${CLP_MODULE_GUI}Lib
    EMSegment
    vtkCommon
    )
#  install(TARGETS ${CLP_MODULE_GUI}Lib
#    RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
#    LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
#    )
endif(EMSEG_GENERATE_CLGUI)
