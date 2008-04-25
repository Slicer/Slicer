project(SlicerBaseCLI)

configure_file(
  ${SlicerBaseCLI_SOURCE_DIR}/vtkSlicerBaseCLIConfigure.h.in 
  ${SlicerBaseCLI_BINARY_DIR}/vtkSlicerBaseCLIConfigure.h
  )

set(include_dirs
  ${SlicerBaseCLI_BINARY_DIR}
  ${SlicerBaseCLI_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

#
# Single library target. Down the road, we may want to have one library that
# only depends on ITK and a second library that only depends on VTK
#
set(SlicerBaseCLI_SRCS 
  vtkPluginFilterWatcher.cxx)
add_library(SlicerBaseCLI ${SlicerBaseCLI_SRCS})
target_link_libraries(SlicerBaseCLI vtkCommon ModuleDescriptionParser)

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} SlicerBaseCLI)

if(BUILD_SHARED_LIBS)
  install(TARGETS SlicerBaseCLI
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

install(FILES 
  itkPluginFilterWatcher.h 
  itkPluginUtilities.h 
  vtkPluginFilterWatcher.h 
  vtkSlicerBaseCLIWin32Header.h
  ${SlicerBaseCLI_BINARY_DIR}/vtkSlicerBaseCLIConfigure.h
  DESTINATION include/Slicer3 COMPONENT Development)

