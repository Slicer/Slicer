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


# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# MRMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).
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
# We should switch to SET_TARGET_PROPERTIES() with LIBRARY_OUTPUT_DIRECTORY 
# once CMake 2.6 is released.  Until then, we need to hack a solution with
# ADD_CUSTOM_COMMAND()
#SET_TARGET_PROPERTIES(CommandIOPlugin PROPERTIES 
#              LIBRARY_OUTPUT_DIRECTORY "${SLICER_LIBRARIES_DIR}/ITKFactories")
GET_TARGET_PROPERTY(LIBFULLNAME CommandIOPlugin LOCATION)
GET_FILENAME_COMPONENT(LIBNAME ${LIBFULLNAME} NAME)

ADD_CUSTOM_COMMAND(TARGET CommandIOPlugin
  DEPENDS ${LIB_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBFULLNAME} ${SLICER_LIBRARIES_DIR}/ITKFactories/${CMAKE_CFG_INTDIR}/${LIBNAME})


IF(BUILD_SHARED_LIBS)
  # Note CommandIO and CommandIOPlugin are installed in different locations
  INSTALL(TARGETS CommandIO  
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
  INSTALL(TARGETS CommandIOPlugin
      RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries)
  INSTALL(TARGETS CommandIOPlugin
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
      ${SLICER_LIBRARIES_DIR}/ITKFactories/libCommandIOPlugin.dylib
      ${SLICER_LIBRARIES_DIR}/ITKFactories/libCommandIOPlugin.so
  )
ENDIF (APPLE)
