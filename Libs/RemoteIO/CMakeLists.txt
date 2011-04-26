project(RemoteIO)

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
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )
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
# --------------------------------------------------------------------------
set(RemoteIO_SRCS
  vtkHTTPHandler.cxx
  vtkSRBHandler.cxx
  vtkXNATHandler.cxx
  vtkHIDHandler.cxx
  vtkXNDHandler.cxx
  vtkHIDTagTable.cxx
  vtkXNDTagTable.cxx
  )

# --------------------------------------------------------------------------
# Build library
# Wrapping
IF (VTK_WRAP_TCL)
  include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  vtk_wrap_tcl3(RemoteIO RemoteIO_TCL_SRCS "${RemoteIO_SRCS}" "")
ENDIF (VTK_WRAP_TCL)

# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})
SET(srcs ${RemoteIO_SRCS})

IF (VTK_WRAP_TCL)
  SET(srcs ${srcs} ${RemoteIO_TCL_SRCS})
ENDIF (VTK_WRAP_TCL)

add_library(${lib_name} ${srcs})
  
SET(libs
  ${SLICERLIBCURL_LIBRARIES}
  MRML
  )
target_link_libraries(${lib_name} ${libs})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF()

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
IF(NOT DEFINED ${lib_name}_INSTALL_BIN_DIR)
  SET(${lib_name}_INSTALL_BIN_DIR bin)
ENDIF()
IF(NOT DEFINED ${lib_name}_INSTALL_LIB_DIR)
  SET(${lib_name}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
ENDIF()

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${lib_name}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${lib_name}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${lib_name}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
IF(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${RemoteIO_SRCS}
    KIT_PYTHON_LIBRARIES MRMLPythonD
    KIT_INSTALL_BIN_DIR ${${lib_name}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${lib_name}_INSTALL_LIB_DIR}
    )
ENDIF()

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  #  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

