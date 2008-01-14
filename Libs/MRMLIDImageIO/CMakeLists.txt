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

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# MRMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).
ADD_LIBRARY(MRMLIDIOPlugin SHARED itkMRMLIDIOPlugin.cxx)
TARGET_LINK_LIBRARIES(MRMLIDIOPlugin MRMLIDIO)
# We should switch to SET_TARGET_PROPERTIES() with LIBRARY_OUTPUT_DIRECTORY 
# once CMake 2.6 is released.  Until then, we need to hack a solution with
# ADD_CUSTOM_COMMAND()
#SET_TARGET_PROPERTIES(MRMLIDIOPlugin PROPERTIES 
#              LIBRARY_OUTPUT_DIRECTORY "${SLICER_LIBRARIES_DIR}/ITKFactories")
GET_TARGET_PROPERTY(LIBFULLNAME MRMLIDIOPlugin LOCATION)
GET_FILENAME_COMPONENT(LIBNAME ${LIBFULLNAME} NAME)

ADD_CUSTOM_COMMAND(TARGET MRMLIDIOPlugin
  DEPENDS ${LIB_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBFULLNAME} ${SLICER_LIBRARIES_DIR}/ITKFactories/${CMAKE_CFG_INTDIR}/${LIBNAME})


IF(BUILD_SHARED_LIBS)
  # Note MRMLIDIO and MRMLIDOPlugin are installed in different locations
  INSTALL(TARGETS MRMLIDIO 
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
  INSTALL(TARGETS MRMLIDIOPlugin
      RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
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
      ${SLICER_LIBRARIES_DIR}/ITKFactories/libMRMLIDIOPlugin.dylib
      ${SLICER_LIBRARIES_DIR}/ITKFactories/libMRMLIDIOPlugin.so
  )
ENDIF (APPLE)
