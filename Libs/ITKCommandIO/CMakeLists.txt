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
  INSTALL(TARGETS CommandIO CommandIOPlugin 
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

CONFIGURE_FILE(
  ${ITKCommandIO_SOURCE_DIR}/itkCommandIOConfigure.h.in 
  ${ITKCommandIO_BINARY_DIR}/itkCommandIOConfigure.h
)

# make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs
IF (APPLE)
  ADD_CUSTOM_COMMAND(
    TARGET CommandIOPlugin
    DEPENDS ${Slicer3_BINARY_DIR}/bin/libCommandIOPlugin.dylib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${Slicer3_BINARY_DIR}/bin/libCommandIOPlugin.dylib
      ${Slicer3_BINARY_DIR}/bin/libCommandIOPlugin.so
  )
ENDIF (APPLE)
