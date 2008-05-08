PROJECT(MRAblation)

# --------------------------------------------------------------------------
# Sources

SET(MRAblation_SRCS
  vtkMRAblationLogic.cxx
  vtkMRAblationGUI.cxx
  vtkMRMLMRAblationNode.cxx
  vtkImageThermalMap.cxx
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
  ${MRAblation_SOURCE_DIR}
  ${MRAblation_BINARY_DIR}
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
VTK_WRAP_TCL3(MRAblation MRAblation_TCL_SRCS "${MRAblation_SRCS}" "")

# --------------------------------------------------------------------------
# Loadable Module support, do after TCL Wrapping

GENERATELM(MRAblation_SRCS MRAblation.txt)

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(MRAblation
  ${MRAblation_SRCS}
  ${MRAblation_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} MRAblation)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(MRAblation
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
  ${MRAblation_SOURCE_DIR}/vtkMRAblationConfigure.h.in 
  ${MRAblation_BINARY_DIR}/vtkMRAblationConfigure.h
)
