project(MRMLIDImageIO)

set(MRMLIDImageIO_SRCS
  itkMRMLIDImageIO.cxx
  itkMRMLIDImageIOFactory.cxx
  )

set(include_dirs
  ${MRMLIDImageIO_SOURCE_DIR}
  ${MRMLIDImageIO_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

add_library(MRMLIDIO ${MRMLIDImageIO_SRCS})
target_link_libraries(MRMLIDIO ITKIO MRML)

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} MRMLIDIO)

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# MRMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).
add_library(MRMLIDIOPlugin SHARED itkMRMLIDIOPlugin.cxx)
target_link_libraries(MRMLIDIOPlugin MRMLIDIO)
# We should switch to SET_TARGET_PROPERTIES() with LIBRARY_OUTPUT_DIRECTORY 
# once CMake 2.6 is released.  Until then, we need to hack a solution with
# ADD_CUSTOM_COMMAND()
#SET_TARGET_PROPERTIES(MRMLIDIOPlugin PROPERTIES 
#              LIBRARY_OUTPUT_DIRECTORY "${SLICER_LIBRARIES_DIR}/ITKFactories")
get_target_property(LIBFULLNAME MRMLIDIOPlugin LOCATION)
get_filename_component(LIBNAME ${LIBFULLNAME} NAME)

add_custom_command(TARGET MRMLIDIOPlugin
  DEPENDS ${LIB_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBFULLNAME} ${SLICER_LIBRARIES_DIR}/ITKFactories/${CMAKE_CFG_INTDIR}/${LIBNAME})


if(BUILD_SHARED_LIBS)
  # Note MRMLIDIO and MRMLIDOPlugin are installed in different locations
  install(TARGETS MRMLIDIO 
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
  install(TARGETS MRMLIDIOPlugin
    RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

configure_file(
  ${MRMLIDImageIO_SOURCE_DIR}/itkMRMLIDImageIOConfigure.h.in 
  ${MRMLIDImageIO_BINARY_DIR}/itkMRMLIDImageIOConfigure.h
  )

# make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs
if(APPLE)
  add_custom_command(
    TARGET MRMLIDIOPlugin
    DEPENDS ${Slicer3_BINARY_DIR}/bin/libMRMLIDIOPlugin.dylib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${SLICER_LIBRARIES_DIR}/ITKFactories/libMRMLIDIOPlugin.dylib
    ${SLICER_LIBRARIES_DIR}/ITKFactories/libMRMLIDIOPlugin.so
    )
endif(APPLE)
