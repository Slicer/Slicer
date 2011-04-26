project(ChangeTrackerCommandLineExecutable)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${ChangeTracker_SOURCE_DIR}
  ${ChangeTracker_SOURCE_DIR}/CommandLineApplication
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
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
  ${ITK_LIBRARIES}
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
  ${ITK_LIBRARIES}
  vtkCommon
  vtkTeem
  )
#slicer3_install_plugins(${CLP_MODULE}Lib)
