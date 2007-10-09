Project(MGHImageIO)

SET(MGHImageIO_SRCS
itkMGHImageIO.cxx
itkMGHImageIOFactory.cxx
)

IF(EXISTS "${ITK_DIR}/CMakeCache.txt")
  INCLUDE_DIRECTORIES("${ITK_DIR}/Utilities/zlib")
ELSE(EXISTS "${ITK_DIR}/CMakeCache.txt")
ENDIF(EXISTS "${ITK_DIR}/CMakeCache.txt")

INCLUDE_DIRECTORIES(
  ${MGHImageIO_SOURCE_DIR}
  ${MGHImageIO_BINARY_DIR}
  ${ITK_SOURCE_DIR}
  ${ITK_BINARY_DIR}
  )


ADD_LIBRARY (MGHIO ${MGHImageIO_SRCS})
TARGET_LINK_LIBRARIES(MGHIO ITKIO MRML)

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add MGHImageIO
# as an ImageIOFactory.  Need to have separate shared library for each new format.
ADD_LIBRARY(MGHIOPlugin SHARED itkMGHIOPlugin.cxx)
TARGET_LINK_LIBRARIES(MGHIOPlugin MGHIO)

# make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs
IF (APPLE)
  ADD_CUSTOM_COMMAND(
    TARGET MGHIOPlugin
    DEPENDS ${Slicer3_BINARY_DIR}/bin/libMGHIOPlugin.dylib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${Slicer3_BINARY_DIR}/bin/libMGHIOPlugin.dylib
      ${Slicer3_BINARY_DIR}/bin/libMGHIOPlugin.so
  )
ENDIF (APPLE)

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS MGHIO MGHIOPlugin
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

CONFIGURE_FILE(
  ${MGHImageIO_SOURCE_DIR}/itkMGHImageIOConfigure.h.in 
  ${MGHImageIO_BINARY_DIR}/itkMGHImageIOConfigure.h
)

