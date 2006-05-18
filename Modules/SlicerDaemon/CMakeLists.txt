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
  ${ModulesBaseClasses_SOURCE_DIR}
  ${ModulesBaseClasses_BINARY_DIR}
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
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} SlicerDaemon)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(SlicerDaemon
  ModulesBaseClasses
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

CONFIGURE_FILE(
  ${SlicerDaemon_SOURCE_DIR}/vtkSlicerDaemonConfigure.h.in 
  ${SlicerDaemon_BINARY_DIR}/vtkSlicerDaemonConfigure.h
)
