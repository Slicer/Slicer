PROJECT(SlicerBaseCLI)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


# --------------------------------------------------------------------------
# Include dirs

SET(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  )

INCLUDE_DIRECTORIES(${include_dirs})

slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerBaseCLIConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseCLIConfigure.h
  )

INSTALL(FILES 
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

SET(SlicerBaseCLI_SRCS
  vtkPluginFilterWatcher.cxx
  )

# --------------------------------------------------------------------------
# Build and install the library

SET(lib_name "SlicerBaseCLI")

ADD_LIBRARY(${lib_name} 
  ${SlicerBaseCLI_SRCS}
  )
TARGET_LINK_LIBRARIES(${lib_name} 
  vtkCommon 
  ModuleDescriptionParser
)

slicer3_get_persistent_property(Slicer3_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES ${tmp} SlicerBaseCLI)

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer3_LIBRARY_PROPERTIES})
ENDIF(Slicer3_LIBRARY_PROPERTIES)

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )

