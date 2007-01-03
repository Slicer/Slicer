PROJECT(SlicerBaseLogic)

# --------------------------------------------------------------------------
# Sources

SET(SlicerBaseLogic_SRCS
  vtkSlicerLogic.cxx 
  vtkSlicerApplicationLogic.cxx 
  vtkSlicerModuleLogic.cxx 
  vtkSlicerTask.cxx
  vtkSlicerMRMLLogic.cxx 
  vtkSlicerSliceLogic.cxx 
  vtkSlicerSliceLayerLogic.cxx 
  vtkSlicerVolumesLogic.cxx 
  vtkSlicerModelsLogic.cxx 
  vtkSlicerFiducialsLogic.cxx
  vtkSlicerColorLogic.cxx
  vtkSlicerIGTDemoLogic.cxx
  # slicer's vtk extensions
  vtkImageSlicePaint.cxx 
  vtkImageAccumulateDiscrete.cxx
  vtkImageBimodalAnalysis.cxx
  vtkImageFillROI.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
  # <classname>.cxx
  #PROPERTIES
  #ABSTRACT "TRUE"
  #)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
  # <classname>.cxx
  #PROPERTIES
  #WRAP_EXCLUDE "TRUE"
  #)

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${VTK_INCLUDE_DIR}  
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${SlicerBase_BINARY_DIR}
  ${OPENTRACKER_INC_DIR}
  )



# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(SlicerBaseLogic SlicerBaseLogic_TCL_SRCS "${SlicerBaseLogic_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(SlicerBaseLogic 
  ${SlicerBaseLogic_SRCS} ${SlicerBaseLogic_TCL_SRCS} 
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} SlicerBaseLogic)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(SlicerBaseLogic 
  MRML
  ${KWWidgets_LIBRARIES}
  vtkITK
  vtkCommonTCL
  vtkFilteringTCL
  vtkGraphicsTCL
  vtkIOTCL
  vtkHybridTCL
  ${OPENTRACKER_LIB}
  )

IF( TEEM_FOUND )
  TARGET_LINK_LIBRARIES(SlicerBaseLogic
    vtkTeem
  )
ENDIF( TEEM_FOUND )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${SlicerBaseLogic_SOURCE_DIR}/vtkSlicerBaseLogicConfigure.h.in 
  ${SlicerBaseLogic_BINARY_DIR}/vtkSlicerBaseLogicConfigure.h
)
