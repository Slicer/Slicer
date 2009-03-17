project(ChangeTrackerCommandLineExecutable)

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
  ${ChangeTracker_SOURCE_DIR}
  ${ChangeTracker_SOURCE_DIR}/CommandLineApplication
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${vtkTeem_SOURCE_DIR} 
  ${vtkTeem_BINARY_DIR}
  )

# --------------------------------------------------------------------------
# Sources

set(CLP_MODULE ChangeTrackerCommandLine)
set(${CLP_MODULE}_SOURCE
  ${CLP_MODULE}.cxx
  )
generateclp(
  ${CLP_MODULE}_SOURCE 
  ${CLP_MODULE}.xml)

# --------------------------------------------------------------------------
# Build and install the exe

add_executable(${CLP_MODULE} ${${CLP_MODULE}_SOURCE})
#slicer3_set_plugins_output_path(${CLP_MODULE})
target_link_libraries(
  ${CLP_MODULE}
  ChangeTracker
  ITKIO 
  ITKCommon 
  vtkCommon
  vtkTeem
  )
install(TARGETS ${CLP_MODULE} RUNTIME DESTINATION bin COMPONENT Development)
#slicer3_install_plugins(${CLP_MODULE})

# this is needed to set up testing
# AF: command line ChangeTracker is only for batch use right now
# -- binary and lib are installed in bin, not pluins
add_library(${CLP_MODULE}Lib SHARED ${${CLP_MODULE}_SOURCE})
set_target_properties (${CLP_MODULE}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
#slicer3_set_plugins_output_path(${CLP_MODULE}Lib)
target_link_libraries(
  ${CLP_MODULE}Lib
  ChangeTracker
  ITKIO 
  ITKCommon 
  vtkCommon
  vtkTeem
  )
#slicer3_install_plugins(${CLP_MODULE}Lib)
