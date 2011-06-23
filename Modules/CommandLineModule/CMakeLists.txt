project(CommandLineModule)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  cmake_policy(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkCommandLineModuleConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkCommandLineModuleConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES
  ${headers}
  "${CMAKE_CURRENT_BINARY_DIR}/vtkCommandLineModuleConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(CommandLineModule_SRCS
  vtkCommandLineModuleLogic.cxx
  vtkCommandLineModuleGUI.cxx
  )

# Abstract/pure virtual classes

#set_source_files_properties(
#vtkModule.cxx
#PROPERTIES
#ABSTRACT "TRUE"
#)

# Helper classes

#set_source_files_properties(
#vtkModule.cxx
#PROPERTIES
#WRAP_EXCLUDE "TRUE"
#)

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(CommandLineModule
  CommandLineModule_TCL_SRCS
  "${CommandLineModule_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name CommandLineModule)
add_library(${lib_name}
  ${CommandLineModule_SRCS}
  ${CommandLineModule_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

# Python settings

if(Slicer_USE_PYTHON)
  find_package(PythonLibs REQUIRED)
  include_directories(${PYTHON_INCLUDE_PATH})
  if(WIN32)
    include_directories(${PYTHON_INCLUDE_PATH}/../PC)
  endif(WIN32)
  if(WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(${lib_name}
      debug ${PYTHON_DEBUG_LIBRARIES}
      optimized ${PYTHON_LIBRARIES})
  else(WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(${lib_name} ${PYTHON_LIBRARIES})
  endif(WIN32 AND PYTHON_DEBUG_LIBRARIES)
  if(NOT WIN32)
    if(APPLE OR UNIX)
      target_link_libraries(${lib_name} ${PYTHON_LIBRARIES})
    else(APPLE OR UNIX)
      target_link_libraries(${lib_name} ${PYTHON_LIBRARIES} util)
    endif(APPLE OR UNIX)
  endif(NOT WIN32)
endif(Slicer_USE_PYTHON)

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
endif(Slicer_LIBRARY_PROPERTIES)

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  #  add_subdirectory(Testing)
endif()
