PROJECT(SlicerDaemon)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
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

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()

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
