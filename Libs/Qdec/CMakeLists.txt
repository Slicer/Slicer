project(Qdec)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include_directories(${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/QdecConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/QdecConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/QdecConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(Qdec_SRCS
  QdecProject.cpp
  QdecContrast.cpp
  QdecDataTable.cpp
  QdecFactor.cpp
  QdecGlmDesign.cpp
  QdecGlmFit.cpp
  QdecGlmFitResults.cpp
  QdecSubject.cpp
  QdecUtilities.cpp
  )

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  vtkQdecNode.cxx 
#  ABSTRACT
#  )

# --------------------------------------------------------------------------
# Wrapping

#INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
#VTK_WRAP_TCL3(Qdec Qdec_TCL_SRCS "${Qdec_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name Qdec)
add_library(${lib_name} 
  ${Qdec_SRCS} 
  #  ${Qdec_TCL_SRCS}
  )
target_link_libraries(${lib_name} 
  vtkIO
  #  vtkCommonTCL
  #  vtkIOTCL
  )

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

# Should the install location change (in both the install and/or build tree)
# do update:
# - vtkQdecModuleLogic.cxx

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkFreeSurferReaders.tcl
  ${CMAKE_BINARY_DIR}/lib/${PROJECT_NAME}/vtkFreeSurferReaders.tcl
  COPYONLY
  )

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/vtkFreeSurferReaders.tcl
  DESTINATION lib/${PROJECT_NAME}
  )
