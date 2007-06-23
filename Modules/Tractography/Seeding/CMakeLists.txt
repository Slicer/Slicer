PROJECT(SlicerTractographySeeding)

# --------------------------------------------------------------------------
# Sources

SET(SlicerTractographySeeding_SRCS
  vtkBSplineInterpolateImageFunction.cxx
  vtkHyperPointandArray.cxx
  vtkHyperStreamlineDTMRI.cxx
  vtkHyperStreamlineTeem.cxx
  #vtkImageGetTensorComponents.cxx
  #vtkImageSetTensorComponents.cxx
  vtkPreciseHyperStreamline.cxx
  vtkPreciseHyperStreamlinePoints.cxx
  vtkSeedTracts.cxx
  vtkTensorImplicitFunctionToFunctionSet.cxx
  vtkTractographyPointAndArray.cxx
)  

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
  #vtkModule.cxx
  #PROPERTIES
  #ABSTRACT "TRUE"
  #)

# Helper classes

SET_SOURCE_FILES_PROPERTIES(
  vtkHyperPointandArray.cxx
  vtkTractographyPointAndArray.cxx
  PROPERTIES
  WRAP_EXCLUDE "TRUE"
  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${SlicerTractographySeeding_SOURCE_DIR}
  ${SlicerTractographySeeding_BINARY_DIR}
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
VTK_WRAP_TCL3(SlicerTractographySeeding SlicerTractographySeeding_TCL_SRCS "${SlicerTractographySeeding_SRCS}" "")


# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(SlicerTractographySeeding
  ${SlicerTractographySeeding_SRCS}
  ${SlicerTractographySeeding_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} SlicerTractographySeeding)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(SlicerTractographySeeding
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
  ${KWWidgets_LIBRARIES}
   )

IF( TEEM_FOUND )
  ADD_DEFINITIONS(-DUSE_TEEM)
  TARGET_LINK_LIBRARIES(SlicerTractographySeeding vtkTeem)
ENDIF( TEEM_FOUND )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

CONFIGURE_FILE(
  ${SlicerTractographySeeding_SOURCE_DIR}/vtkSlicerTractographySeedingConfigure.h.in 
  ${SlicerTractographySeeding_BINARY_DIR}/vtkSlicerTractographySeedingConfigure.h
)
