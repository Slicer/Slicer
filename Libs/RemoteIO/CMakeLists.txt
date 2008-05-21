project(RemoteIO)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

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

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

foreach(package 
    MRML
    )
  # This is mostly to test building this lib as standalone outside Slicer3
  if(NOT DEFINED ${package}_SOURCE_DIR)
    set(${package}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${package}")
  endif(NOT DEFINED ${package}_SOURCE_DIR)
  if(NOT DEFINED ${package}_BINARY_DIR)
    string(REPLACE ${PROJECT_NAME} ${package} 
      ${package}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    link_directories(${${package}_BINARY_DIR})
  endif(NOT DEFINED ${package}_BINARY_DIR)

  set(include_dirs ${include_dirs}
    ${${package}_SOURCE_DIR}
    ${${package}_BINARY_DIR}
    )
endforeach(package)

include_directories(${include_dirs})

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
  itksys
  ${SLICERLIBCURL_LIBRARIES}
  MRML
  )

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

