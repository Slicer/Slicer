project(CollectFiducials)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkCollectFiducialsConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkCollectFiducialsConfigure.h
  )

file(GLOB headers
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES
  ${headers}
  "${CMAKE_CURRENT_BINARY_DIR}/vtkCollectFiducialsConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(CollectFiducials_SRCS
  vtkCollectFiducialsLogic.cxx
  vtkCollectFiducialsGUI.cxx
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(CollectFiducials CollectFiducials_TCL_SRCS "${CollectFiducials_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(CollectFiducials_SRCS CollectFiducials.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name CollectFiducials)
add_library(${lib_name}
  ${CollectFiducials_SRCS}
  ${CollectFiducials_TCL_SRCS}
  )

slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

slicer3_install_modules(${lib_name})
