project(Qdec)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS ${tmp} ${include_dirs})

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

slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES ${tmp} ${lib_name})

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

# IF(BUILD_TESTING)
#  SUBDIRS(Testing)
# ENDIF(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkFreeSurferReaders.tcl
  ${Slicer3_BINARY_DIR}/lib/${PROJECT_NAME}/Tcl/vtkFreeSurferReaders.tcl
  COPYONLY
  )

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/vtkFreeSurferReaders.tcl
  DESTINATION lib/${PROJECT_NAME}/Tcl
  )
