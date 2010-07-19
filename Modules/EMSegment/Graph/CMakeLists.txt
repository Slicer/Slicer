project(EMSegmentGraph)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentGraphConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkEMSegmentGraphConfigure.h
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkEMSegmentGraphConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(KIT_SRCS 
  vtkImageGraph.cxx
  vtkIndirectLookupTable.cxx
  vtkImageDrawObjects.cxx
  vtkImageCurveRegion.cxx
  )
  
# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(${PROJECT_NAME} 
  KIT_TCL_SRCS 
  "${KIT_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name ${PROJECT_NAME})
add_library(${lib_name} 
  ${KIT_SRCS} 
  ${KIT_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
    ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install the testing data

#install(DIRECTORY
#  Testing/TestData
#  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
#  PATTERN ".svn" EXCLUDE
#  )
