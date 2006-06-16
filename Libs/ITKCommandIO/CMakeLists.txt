PROJECT(ITKCommandIO)

SET(ITKCOMMANDIO_SRCS itkCommandIO.cxx itkCommandIOFactory.cxx)

#
# Library for all of my image formats
ADD_LIBRARY(CommandIO ${ITKCOMMANDIO_SRCS})
TARGET_LINK_LIBRARIES(CommandIO ITKCommon slicerio ITKNrrdIO)

#
# Shared library that when placed in ITK_AUTOLOAD_PATH, will add MyImageIO
# as an ImageIOFactory.  Need to have separate shared library for each new format.
ADD_LIBRARY(CommandIOPlugin SHARED itkCommandIOPlugin.cxx)
TARGET_LINK_LIBRARIES(CommandIOPlugin CommandIO)

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} CommandIO CommandIOPlugin)
ENDIF(BUILD_SHARED_LIBS)

OPTION (MY_DEBUG
  "Building debug code"
ON)
IF(MY_DEBUG)
  SET_SOURCE_FILES_PROPERTIES(
    itkCommandIO.cxx itkCommandIOFactory.cxx 
  COMPILE_FLAGS -g)
ENDIF(MY_DEBUG)
