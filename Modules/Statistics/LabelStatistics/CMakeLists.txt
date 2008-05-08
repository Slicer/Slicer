PROJECT(LabelStatistics)
OPTION(LABELSTATISTICS "Do you want to build the LabelStatistics Module?" ON)
IF(LABELSTATISTICS)

# --------------------------------------------------------------------------
# Sources

SET(LabelStatistics_SRCS
  vtkLabelStatisticsLogic.cxx
  vtkLabelStatisticsGUI.cxx
  vtkMRMLLabelStatisticsNode.cxx
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
  ${LabelStatistics_SOURCE_DIR}
  ${LabelStatistics_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )


# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(LabelStatistics LabelStatistics_TCL_SRCS "${LabelStatistics_SRCS}" "")


# --------------------------------------------------------------------------
# Loadable Module
GENERATELM(LabelStatistics_SRCS LabelStatistics.txt)

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(LabelStatistics
  ${LabelStatistics_SRCS}
  ${LabelStatistics_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} LabelStatistics)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(LabelStatistics
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  ${KWWidgets_LIBRARIES} )



# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${LabelStatistics_SOURCE_DIR}/vtkLabelStatisticsConfigure.h.in 
  ${LabelStatistics_BINARY_DIR}/vtkLabelStatisticsConfigure.h
)

ENDIF(LABELSTATISTICS)
