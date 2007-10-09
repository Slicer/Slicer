PROJECT(GradientAnisotropicDiffusionFilter)

# --------------------------------------------------------------------------
# Sources

SET(GradientAnisotropicDiffusionFilter_SRCS
  vtkGradientAnisotropicDiffusionFilterLogic.cxx
  vtkGradientAnisotropicDiffusionFilterGUI.cxx
  vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx
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
  ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}
  ${GradientAnisotropicDiffusionFilter_BINARY_DIR}
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
VTK_WRAP_TCL3(GradientAnisotropicDiffusionFilter GradientAnisotropicDiffusionFilter_TCL_SRCS "${GradientAnisotropicDiffusionFilter_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(GradientAnisotropicDiffusionFilter
  ${GradientAnisotropicDiffusionFilter_SRCS}
  ${GradientAnisotropicDiffusionFilter_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS GradientAnisotropicDiffusionFilter
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(GradientAnisotropicDiffusionFilter
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
  ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h.in 
  ${GradientAnisotropicDiffusionFilter_BINARY_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h
)
