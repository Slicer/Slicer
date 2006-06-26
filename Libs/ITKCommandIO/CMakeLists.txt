PROJECT(ITKCommandIO)

SET(ITKCOMMANDIO_SRCS itkCommandIO.cxx itkCommandIOFactory.cxx)

INCLUDE_DIRECTORIES(
  ${ITKCommandIO_SOURCE_DIR}
  ${ITKCommandIO_BINARY_DIR}
  )
#
# Library for all of my image formats
ADD_LIBRARY(CommandIO ${ITKCOMMANDIO_SRCS})
TARGET_LINK_LIBRARIES(CommandIO 
  ITKIO
)

  
#
# Shared library that when placed in ITK_AUTOLOAD_PATH, will add MyImageIO
# as an ImageIOFactory.  Need to have separate shared library for each new format.
ADD_LIBRARY(CommandIOPlugin SHARED itkCommandIOPlugin.cxx)
TARGET_LINK_LIBRARIES(CommandIOPlugin CommandIO  
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  slicerio ITKNrrdIO)

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} CommandIO CommandIOPlugin)
ENDIF(BUILD_SHARED_LIBS)

CONFIGURE_FILE(
  ${ITKCommandIO_SOURCE_DIR}/itkCommandIOConfigure.h.in 
  ${ITKCommandIO_BINARY_DIR}/itkCommandIOConfigure.h
)
