PROJECT(SlicerBaseCLI)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()


# --------------------------------------------------------------------------
# Include dirs

SET(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  )

INCLUDE_DIRECTORIES(${include_dirs})

# Update Slicer_Base_INCLUDE_DIRS
SET(Slicer_Base_INCLUDE_DIRS ${Slicer_Base_INCLUDE_DIRS} ${include_dirs} CACHE INTERNAL "Slicer Base includes" FORCE)

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
  DESTINATION ${Slicer_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
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
SET_TARGET_PROPERTIES(${lib_name} PROPERTIES LABELS ${lib_name})

# Update Slicer_Base_LIBRARIES
SET(Slicer_Base_LIBRARIES ${Slicer_Base_LIBRARIES} ${lib_name} CACHE INTERNAL "Slicer Base libraries" FORCE)

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF(Slicer_LIBRARY_PROPERTIES)

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

