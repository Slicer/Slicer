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

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# MRMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).
ADD_LIBRARY(MGHIOPlugin SHARED itkMGHIOPlugin.cxx)
TARGET_LINK_LIBRARIES(MGHIOPlugin MGHIO)
# We should switch to SET_TARGET_PROPERTIES() with LIBRARY_OUTPUT_DIRECTORY 
# once CMake 2.6 is released.  Until then, we need to hack a solution with
# ADD_CUSTOM_COMMAND()
#SET_TARGET_PROPERTIES(MGHIOPlugin PROPERTIES 
#              LIBRARY_OUTPUT_DIRECTORY "${SLICER_LIBRARIES_DIR}/ITKFactories")
GET_TARGET_PROPERTY(LIBFULLNAME MGHIOPlugin LOCATION)
GET_FILENAME_COMPONENT(LIBNAME ${LIBFULLNAME} NAME)

ADD_CUSTOM_COMMAND(TARGET MGHIOPlugin
  DEPENDS ${LIB_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBFULLNAME} ${SLICER_LIBRARIES_DIR}/ITKFactories/${CMAKE_CFG_INTDIR}/${LIBNAME})


# make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs
IF (APPLE)
  ADD_CUSTOM_COMMAND(
    TARGET MGHIOPlugin
    DEPENDS ${Slicer3_BINARY_DIR}/bin/libMGHIOPlugin.dylib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${SLICER_LIBRARIES_DIR}/ITKFactories/libMGHIOPlugin.dylib
      ${SLICER_LIBRARIES_DIR}/ITKFactories/libMGHIOPlugin.so
  )
ENDIF (APPLE)

IF(BUILD_SHARED_LIBS)
  # Note MGHIO and MGHIOPlugin are installed in different locations
  INSTALL(TARGETS MGHIO 
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
  INSTALL(TARGETS MGHIOPlugin
      RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

CONFIGURE_FILE(
  ${MGHImageIO_SOURCE_DIR}/itkMGHImageIOConfigure.h.in 
  ${MGHImageIO_BINARY_DIR}/itkMGHImageIOConfigure.h
)

