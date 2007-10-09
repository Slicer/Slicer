Project(MRMLIDImageIO)

SET(MRMLIDImageIO_SRCS
itkMRMLIDImageIO.cxx
itkMRMLIDImageIOFactory.cxx
)

INCLUDE_DIRECTORIES(
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${MRMLIDImageIO_SOURCE_DIR}
  ${MRMLIDImageIO_BINARY_DIR}
  )


ADD_LIBRARY (MRMLIDIO ${MRMLIDImageIO_SRCS})
TARGET_LINK_LIBRARIES(MRMLIDIO ITKIO MRML)

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add MRMLIDImageIO
# as an ImageIOFactory.  Need to have separate shared library for each new format.
ADD_LIBRARY(MRMLIDIOPlugin SHARED itkMRMLIDIOPlugin.cxx)
TARGET_LINK_LIBRARIES(MRMLIDIOPlugin MRMLIDIO)

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS MRMLIDIO MRMLIDIOPlugin 
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

CONFIGURE_FILE(
  ${MRMLIDImageIO_SOURCE_DIR}/itkMRMLIDImageIOConfigure.h.in 
  ${MRMLIDImageIO_BINARY_DIR}/itkMRMLIDImageIOConfigure.h
)

# make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs
IF (APPLE)
  ADD_CUSTOM_COMMAND(
    TARGET MRMLIDIOPlugin
    DEPENDS ${Slicer3_BINARY_DIR}/bin/libMRMLIDIOPlugin.dylib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${Slicer3_BINARY_DIR}/bin/libMRMLIDIOPlugin.dylib
      ${Slicer3_BINARY_DIR}/bin/libMRMLIDIOPlugin.so
  )
ENDIF (APPLE)
