PROJECT(SlicerDaemon)

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
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerDaemonConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerDaemonConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerDaemonConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(SlicerDaemon_SRCS
  #vtkSlicerDaemonLogic.cxx
  #vtkSlicerDaemonGUI.cxx
  #vtkMRMLSlicerDaemonNode.cxx
  vtkTclHelper.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(SlicerDaemon SlicerDaemon_TCL_SRCS "${SlicerDaemon_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name SlicerDaemon)
add_library(${lib_name}
  ${SlicerDaemon_SRCS}
  ${SlicerDaemon_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

# Matlab

file(GLOB MFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Matlab/*.m")
file(GLOB CFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Matlab/popen/*.c")

foreach(file 
    ${MFILES} 
    ${CFILES}
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(DIRECTORY
  Matlab
  DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )

# Python

file(GLOB PYTHONFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Python/*.py")
foreach(pythonfile ${PYTHONFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${pythonfile}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${pythonfile}
    COPYONLY)
endforeach(pythonfile)

install(
  FILES ${PYTHONFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/Python
  )

# Tcl

file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tcl/*.tcl")
file(GLOB TRACKERFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tcl/tracker/*.[mtv]*")
foreach(file 
    ${TCLFILES} 
    ${TRACKERFILES}
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(DIRECTORY
  Tcl
  DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )

# custom install for slicerget/slicerput utility programs
# that can't work. ${Slicer_INSTALL_BIN_DIR} is relative.
# It's either a wrong CONFIGURE, or a should have been an INSTALL, who knows
#configure_file(
#  ${CMAKE_CURRENT_SOURCE_DIR}/Tcl/slicerget.tcl
#  ${Slicer_INSTALL_BIN_DIR}/slicerget
#  COPYONLY
#  )
#configure_file(
#  ${CMAKE_CURRENT_SOURCE_DIR}/Tcl/slicerput.tcl
#  ${Slicer_INSTALL_BIN_DIR}/slicerput 
#  COPYONLY
#  )

# Testing

file(GLOB MRMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.mrml")
file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/Tcl/*.tcl")

foreach(file 
    ${TCLFILES} 
    ${MRMLFILES}
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(DIRECTORY
  Testing
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
