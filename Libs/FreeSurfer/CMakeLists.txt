project(FreeSurfer)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/FreeSurferConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/FreeSurferConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(
  FILES ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/FreeSurferConfigure.h" 
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(FreeSurfer_SRCS
  vtkFSIO.cxx
  vtkFSSurfaceReader.cxx
  vtkFSSurfaceAnnotationReader.cxx
  vtkFSSurfaceScalarReader.cxx
  vtkFSSurfaceWFileReader.cxx
  vtkFSSurfaceLabelReader.cxx
  vtkFSLookupTable.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(FreeSurfer FreeSurfer_TCL_SRCS "${FreeSurfer_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name FreeSurfer)
add_library(${lib_name} 
  ${FreeSurfer_SRCS} 
  ${FreeSurfer_TCL_SRCS}
  )

target_link_libraries(${lib_name} 
  vtkIO
  vtkCommonTCL
  vtkIOTCL
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Install support files

# Should this location change, update Libs\MRML\vtkMRMLFreeSurferProceduralColorNode.cxx

# Copy the resource files to the build directory

set(RESFILES
  FreeSurferColorLUT.txt
  Simple_surface_labels2002.txt
  )
foreach(file ${RESFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/share/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file)
install(
  FILES ${RESFILES}
  DESTINATION share/${PROJECT_NAME}
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

# IF(BUILD_TESTING)
#  SUBDIRS(Testing)
# ENDIF(BUILD_TESTING)

# Install the testing data

file(GLOB_RECURSE PNGFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.png")
file(GLOB_RECURSE TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.tcl")
file(GLOB_RECURSE CURVFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.curv")
file(GLOB_RECURSE ORIGFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.orig")

foreach(file 
    ${PNGFILES}
    ${TCLFILES}
    ${CURVFILES}
    ${ORIGFILES}
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/share/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file) 

install(DIRECTORY
  Testing
  DESTINATION share/${PROJECT_NAME} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
