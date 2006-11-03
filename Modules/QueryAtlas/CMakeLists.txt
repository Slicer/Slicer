PROJECT(QueryAtlas)

# --------------------------------------------------------------------------
# Sources

SET(QueryAtlas_SRCS
  vtkQueryAtlasLogic.cxx
  vtkQueryAtlasGUI.cxx
  vtkMRMLQueryAtlasNode.cxx
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
  ${QueryAtlas_SOURCE_DIR}
  ${QueryAtlas_BINARY_DIR}
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
VTK_WRAP_TCL3(QueryAtlas QueryAtlas_TCL_SRCS "${QueryAtlas_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(QueryAtlas
  ${QueryAtlas_SRCS}
  ${QueryAtlas_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} QueryAtlas)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(QueryAtlas
  ModulesBaseClasses
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
  ${QueryAtlas_SOURCE_DIR}/vtkQueryAtlasConfigure.h.in 
  ${QueryAtlas_BINARY_DIR}/vtkQueryAtlasConfigure.h
)
