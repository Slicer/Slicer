project(RemoteIO)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(SLICERLIBCURL REQUIRED)
include(${SLICERLIBCURL_USE_FILE})

# Disable MSVC 8 warnings
if(WIN32)
  option(DISABLE_MSVC8_DEPRECATED_WARNINGS 
    "Disable Visual Studio 8 deprecated warnings" ON)
  mark_as_advanced(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  if(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif(DISABLE_MSVC8_DEPRECATED_WARNINGS)
endif(WIN32)

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkRemoteIOConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkRemoteIOConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkRemoteIOConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(RemoteIO_SRCS
  vtkHTTPHandler.cxx
  vtkSRBHandler.cxx
  vtkXNATHandler.cxx
  )

# --------------------------------------------------------------------------
# Wrapping
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(RemoteIO RemoteIO_TCL_SRCS "${RemoteIO_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name RemoteIO)
add_library(${lib_name}
  ${RemoteIO_SRCS} 
  ${RemoteIO_TCL_SRCS}
  )
target_link_libraries(${lib_name}
  vtkIO
  vtkIOTCL
  vtkCommonTCL
  ${SLICERLIBCURL_LIBRARIES}
  MRML
  )

# RemoteIO is linked against a STATIC libcurl, which causes some problem
# for external project and CMake 2.4 to link against since they assume 
# it's SHARED. Fixed in 2.6, let's comment in the meantime.
#slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES tmp)
#slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES ${tmp} ${lib_name})

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  #  SUBDIRS(Testing)
endif(BUILD_TESTING)

