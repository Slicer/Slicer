project(SlicerBaseCLI)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerBaseCLIConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseCLIConfigure.h
  )

install(FILES 
  itkPluginFilterWatcher.h 
  itkPluginUtilities.h 
  vtkPluginFilterWatcher.h 
  vtkSlicerBaseCLIWin32Header.h
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseCLIConfigure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

# Single library target. Down the road, we may want to have one library that
# only depends on ITK and a second library that only depends on VTK

set(SlicerBaseCLI_SRCS 
  vtkPluginFilterWatcher.cxx
  )

# --------------------------------------------------------------------------
# Build and install the library

add_library(SlicerBaseCLI 
  ${SlicerBaseCLI_SRCS}
  )
target_link_libraries(SlicerBaseCLI 
  vtkCommon 
  ModuleDescriptionParser
)

slicer3_get_persistent_property(Slicer3_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES ${tmp} SlicerBaseCLI)

install(TARGETS SlicerBaseCLI
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )

