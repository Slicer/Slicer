PROJECT(SlicerBaseCLI)

CONFIGURE_FILE(
  ${SlicerBaseCLI_SOURCE_DIR}/vtkSlicerBaseCLIConfigure.h.in 
  ${SlicerBaseCLI_BINARY_DIR}/vtkSlicerBaseCLIConfigure.h
)


INCLUDE_DIRECTORIES(
  ${ModuleDescriptionParser_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${SlicerBaseCLI_BINARY_DIR}
  ${SlicerBaseCLI_SOURCE_DIR}
)

#
# Single library target. Down the road, we may want to have one library that only depends 
# on ITK and a second library that only depends on VTK
#
SET(SlicerBaseCLI_SRCS 
      vtkPluginFilterWatcher.cxx)
ADD_LIBRARY(SlicerBaseCLI ${SlicerBaseCLI_SRCS})
TARGET_LINK_LIBRARIES(SlicerBaseCLI vtkCommon ModuleDescriptionParser)


IF(BUILD_SHARED_LIBS)
   INSTALL(TARGETS SlicerBaseCLI
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

INSTALL(FILES itkPluginFilterWatcher.h itkPluginUtilities.h vtkPluginFilterWatcher.h vtkSlicerBaseCLIWin32Header.h
        ${SlicerBaseCLI_BINARY_DIR}/vtkSlicerBaseCLIConfigure.h
        DESTINATION include/Slicer3 COMPONENT Development)

