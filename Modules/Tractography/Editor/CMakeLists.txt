project(SlicerTractographyROISelect)

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
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Sources

set(CLP ROISelect)
set(${CLP}_SOURCE ${CLP}.cxx)

generateclp(
  ${CLP}_SOURCE 
  ${CLP}.xml 
  ${Slicer3_HOME}/${Slicer3_INSTALL_SHARE_DIR}/Resources/NAMICLogo.h
)

# --------------------------------------------------------------------------
# Build and install the exe

add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries(${CLP}
  ${Slicer3_Libs_LIBRARIES}
  vtkIO
  ${ITK_LIBRARIES}
  )

slicer3_install_plugins(${CLP})
