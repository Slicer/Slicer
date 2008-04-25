project(RemoteIO)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
configure_file(
  ${RemoteIO_SOURCE_DIR}/vtkRemoteIOConfigure.h.in 
  ${RemoteIO_BINARY_DIR}/vtkRemoteIOConfigure.h
  )

#
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
# Settings for options
set(RemoteIO_SRCS
  vtkHTTPHandler.cxx
  vtkSRBHandler.cxx
  vtkXNATHandler.cxx
  )

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${RemoteIO_SOURCE_DIR}
  ${RemoteIO_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

# --------------------------------------------------------------------------
# Find SLICERLIBCURL (slicercurl)
find_package(SLICERLIBCURL REQUIRED)
include(${SLICERLIBCURL_USE_FILE})

# --------------------------------------------------------------------------
# Wrapping
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(RemoteIO RemoteIO_TCL_SRCS "${RemoteIO_SRCS}" "")

# Build the library
add_library(RemoteIO
  ${RemoteIO_SRCS} ${RemoteIO_TCL_SRCS}
  )

# RemoteIO is linked against a STATIC libcurl, which causes some problem
# for external project and CMake 2.4 to link against since they assume 
# it's SHARED. Fixed in 2.6, let's comment in the meantime.
#GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
#SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} RemoteIO)

if(BUILD_SHARED_LIBS)
  install(TARGETS RemoteIO
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

target_link_libraries(RemoteIO
  vtkIO
  vtkIOTCL
  vtkCommonTCL
  ${SLICERLIBCURL_LIBRARIES}
  MRML
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  #  SUBDIRS(Testing)
endif(BUILD_TESTING)

