PROJECT(IGT)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
CONFIGURE_FILE(
  ${IGT_SOURCE_DIR}/vtkIGTConfigure.h.in 
  ${IGT_BINARY_DIR}/vtkIGTConfigure.h
)


# --------------------------------------------------------------------------
# Settings for options
SET(IGT_SRCS
  vtkIGTDataManager.cxx
  vtkIGTDataStream.cxx
  vtkIGTMatrixState.cxx
  vtkIGTPat2ImgRegistration.cxx
)
SET(TRACKING_INTERFACE_LIBRARY "")
IF(USE_NAVITRACK)
  SET(IGT_SRCS
    vtkIGTDataManager.cxx
    vtkIGTDataStream.cxx
    vtkIGTMatrixState.cxx
    vtkIGTPat2ImgRegistration.cxx
    vtkIGTOpenTrackerStream.cxx
    vtkIGTMessageAttributeBase.cxx
    vtkIGTMessageAttributeSet.cxx
#    vtkIGTMessageGenericAttribute.cxx
    vtkIGTMessageImageDataAttribute.cxx
  )
  SET(TRACKING_INTERFACE_INCLUDE_DIR
    ${NAVITRACK_INC}
    ${NAVITRACK_INC}/../libs
  )
  SET(TRACKING_INTERFACE_LIB  
    ${NAVITRACK_LIB}
  )
ENDIF(USE_NAVITRACK)

IF(USE_IGSTK)
  SET(IGT_SRCS
    vtkIGTDataManager.cxx
    vtkIGTDataStream.cxx
    vtkIGTMatrixState.cxx
    vtkIGTPat2ImgRegistration.cxx
    vtkIGTIGSTKStream.cxx
  )
  SET(TRACKING_INTERFACE_INCLUDE_DIR
    ${IGSTK_INCLUDE_DIRS}
  )
  SET(TRACKING_INTERFACE_LIB  
    ${IGSTK_LIBRARIES}
  )
ENDIF(USE_IGSTK)


# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${MRML_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${IGT_BINARY_DIR}
  ${IGT_SOURCE_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${TRACKING_INTERFACE_INCLUDE_DIR}
  )


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
  INSTALL(TARGETS IGT
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
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
  ${TRACKING_INTERFACE_LIB}
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

