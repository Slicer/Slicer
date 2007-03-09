PROJECT(IGT)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
CONFIGURE_FILE(
  ${IGT_SOURCE_DIR}/vtkIGTConfigure.h.in 
  ${IGT_BINARY_DIR}/vtkIGTConfigure.h
)


# --------------------------------------------------------------------------
# Sources

IF(USE_OPENTRACKER)
SET(IGT_SRCS
    vtkIGTDataManager.cxx
    vtkIGTDataStream.cxx
    vtkIGTMatrixState.cxx
    vtkIGTOpenTrackerStream.cxx
    vtkIGTPat2ImgRegistration.cxx
)
ELSE(USE_OPENTRACKER)
SET(IGT_SRCS
    vtkIGTDataManager.cxx
    vtkIGTDataStream.cxx
    vtkIGTMatrixState.cxx
    vtkIGTPat2ImgRegistration.cxx
)
ENDIF(USE_OPENTRACKER)




# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
    #  ${SlicerBaseGUI_SOURCE_DIR}
    # ${SlicerBaseGUI_BINARY_DIR}
    # ${SlicerBaseLogic_SOURCE_DIR}
    # ${SlicerBaseLogic_BINARY_DIR}
  ${MRML_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${IGT_BINARY_DIR}
  ${IGT_SOURCE_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}

  )

IF(USE_OPENTRACKER)
INCLUDE_DIRECTORIES(
  ${OPENTRACKER_INC_DIR}
)
ENDIF(USE_OPENTRACKER)

# --------------------------------------------------------------------------
# Wrapping
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(IGT IGT_TCL_SRCS "${IGT_SRCS}" "")


# Build the library

ADD_LIBRARY(IGT 
  ${IGT_SRCS} ${IGT_TCL_SRCS}
  )
IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} IGT)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(IGT
  MRML
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkHybridTCL
  vtkIOTCL
  vtkGraphicsTCL
  vtkRenderingTCL
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  ${KWWidgets_LIBRARIES}
  ${OPENTRACKER_LIB}
  )




# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

