PROJECT(SlicerDaemon)

# --------------------------------------------------------------------------
# Sources

SET(SlicerDaemon_SRCS
  #vtkSlicerDaemonLogic.cxx
  #vtkSlicerDaemonGUI.cxx
  #vtkMRMLSlicerDaemonNode.cxx
  vtkTclHelper.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
  #vtkModule.cxx
  #PROPERTIES
  #ABSTRACT "TRUE"
  #)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
  #vtkModule.cxx
  #PROPERTIES
  #WRAP_EXCLUDE "TRUE"
  #)

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${SlicerDaemon_SOURCE_DIR}
  ${SlicerDaemon_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )


# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(SlicerDaemon SlicerDaemon_TCL_SRCS "${SlicerDaemon_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(SlicerDaemon
  ${SlicerDaemon_SRCS}
  ${SlicerDaemon_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS SlicerDaemon
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(SlicerDaemon
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkCommonTCL
  vtkImagingTCL
  ${KWWidgets_LIBRARIES} )



# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

#
# configure files to drop in the build directory
#
CONFIGURE_FILE(
  ${SlicerDaemon_SOURCE_DIR}/vtkSlicerDaemonConfigure.h.in 
  ${SlicerDaemon_BINARY_DIR}/vtkSlicerDaemonConfigure.h
)

CONFIGURE_FILE(
  ${SlicerDaemon_SOURCE_DIR}/Tcl/slicerd.tcl
  ${SLICER_LIBRARIES_DIR}/slicerd.tcl
)

CONFIGURE_FILE(
  ${SlicerDaemon_SOURCE_DIR}/Tcl/slicerget.tcl
  ${SLICER_LIBRARIES_DIR}/slicerget.tcl
)

CONFIGURE_FILE(
  ${SlicerDaemon_SOURCE_DIR}/Tcl/slicerput.tcl
  ${SLICER_LIBRARIES_DIR}/slicerput.tcl
)

INSTALL(FILES
  "${SLICER_LIBRARIES_DIR}/slicerd.tcl"
  DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR})

INSTALL(FILES
  "${SLICER_LIBRARIES_DIR}/slicerput.tcl"
  DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR})

INSTALL(FILES
  "${SLICER_LIBRARIES_DIR}/slicerget.tcl"
  DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR})


