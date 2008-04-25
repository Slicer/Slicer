project(ITKCommandIO)

set(ITKCOMMANDIO_SRCS itkCommandIO.cxx itkCommandIOFactory.cxx)

set(include_dirs
  ${ITKCommandIO_SOURCE_DIR}
  ${ITKCommandIO_BINARY_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

# Library for all of my image formats
add_library(CommandIO ${ITKCOMMANDIO_SRCS})
target_link_libraries(CommandIO 
  ITKIO
  )

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} CommandIO)

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# MRMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).
add_library(CommandIOPlugin SHARED itkCommandIOPlugin.cxx)
target_link_libraries(CommandIOPlugin CommandIO  
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
get_target_property(LIBFULLNAME CommandIOPlugin LOCATION)
get_filename_component(LIBNAME ${LIBFULLNAME} NAME)

add_custom_command(TARGET CommandIOPlugin
  DEPENDS ${LIB_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBFULLNAME} ${SLICER_LIBRARIES_DIR}/ITKFactories/${CMAKE_CFG_INTDIR}/${LIBNAME})

if(BUILD_SHARED_LIBS)
  # Note CommandIO and CommandIOPlugin are installed in different locations
  install(TARGETS CommandIO  
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
  install(TARGETS CommandIOPlugin
    RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

configure_file(
  ${ITKCommandIO_SOURCE_DIR}/itkCommandIOConfigure.h.in 
  ${ITKCommandIO_BINARY_DIR}/itkCommandIOConfigure.h
  )

# make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs
if(APPLE)
  add_custom_command(
    TARGET CommandIOPlugin
    DEPENDS ${Slicer3_BINARY_DIR}/bin/libCommandIOPlugin.dylib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${SLICER_LIBRARIES_DIR}/ITKFactories/libCommandIOPlugin.dylib
    ${SLICER_LIBRARIES_DIR}/ITKFactories/libCommandIOPlugin.so
    )
endif(APPLE)
