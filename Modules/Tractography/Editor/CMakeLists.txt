project(SlicerTractographyROISelect)

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
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Sources

set(CLP ROISelect)
set(${CLP}_SOURCE ${CLP}.cxx)

generateclp(
  ${CLP}_SOURCE 
  ${CLP}.xml 
  ${Slicer_HOME}/${Slicer_INSTALL_SHARE_DIR}/Resources/NAMICLogo.h
)

# --------------------------------------------------------------------------
# Build and install the exe

add_executable(${CLP} ${${CLP}_SOURCE})
slicer3_set_plugins_output_path(${CLP})
target_link_libraries(${CLP}
  ${Slicer_Libs_LIBRARIES}
  vtkIO
  ${ITK_LIBRARIES}
  )

slicer3_install_plugins(${CLP})
