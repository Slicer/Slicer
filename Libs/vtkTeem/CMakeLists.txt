PROJECT(vtkTeem)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# --------------------------------------------------------------------------
# Sources

# include a test for endianness (used to be done by default)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMAKE_WORDS_BIGENDIAN)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})


SET(vtkTeem_SRCS
  vtkNRRDReader.cxx
  vtkNRRDWriter.cxx
  vtkDiffusionTensorMathematics.cxx
  vtkDiffusionTensorMathematicsSimple.cxx
  vtkTeemEstimateDiffusionTensor.cxx
  vtkDiffusionTensorGlyph.cxx
  vtkPolyDataTensorToColor.cxx

  vtkBSplineInterpolateImageFunction.cxx
  vtkHyperPointandArray.cxx
  vtkHyperStreamlineDTMRI.cxx
  vtkHyperStreamlineTeem.cxx
  vtkPreciseHyperStreamline.cxx
  vtkPreciseHyperStreamlinePoints.cxx
  vtkSeedTracts.cxx
  vtkTensorImplicitFunctionToFunctionSet.cxx
  vtkTractographyPointAndArray.cxx
  vtkTensorMask.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  ABSTRACT
#  )

# Helper classes

SET_SOURCE_FILES_PROPERTIES(
  vtkHyperPointandArray.cxx
  vtkTractographyPointAndArray.cxx
  WRAP_EXCLUDE
  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${TEEM_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(vtkTeem vtkTeem_TCL_SRCS "${vtkTeem_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(vtkTeem 
  ${vtkTeem_SRCS} 
  ${vtkTeem_TCL_SRCS} 
  )

IF(Slicer3_SOURCE_DIR)
  IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS vtkTeem
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(Slicer3_SOURCE_DIR)

# after teem 1.9, all the teem libraries are groups in one big libteem.so
IF ( ${TEEM_VERSION_MAJOR} GREATER 1 )
  SET(TEEM_LIBRARIES teem)
ELSE ( ${TEEM_VERSION_MAJOR} GREATER 1 )
  IF ( ${TEEM_VERSION_MINOR} LESS 10 )
    SET(TEEM_LIBRARIES air biff nrrd ten ell)
  ELSE ( ${TEEM_VERSION_MINOR} LESS 10 )
    SET(TEEM_LIBRARIES teem)
  ENDIF ( ${TEEM_VERSION_MINOR} LESS 10 )
ENDIF ( ${TEEM_VERSION_MAJOR} GREATER 1 )

TARGET_LINK_LIBRARIES(vtkTeem 
  vtkCommon
  vtkFiltering
  vtkImagingTCL
  vtkIOTCL
  vtkGraphicsTCL
  ${TEEM_LIBRARIES}
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${vtkTeem_SOURCE_DIR}/vtkTeemConfigure.h.in 
  ${vtkTeem_BINARY_DIR}/vtkTeemConfigure.h
)

CONFIGURE_FILE(
  ${vtkTeem_SOURCE_DIR}/vtkTeemConfig.cmake.in 
  ${vtkTeem_BINARY_DIR}/vtkTeemConfig.cmake
)
