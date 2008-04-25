project(MGHImageIO)

set(MGHImageIO_SRCS
  itkMGHImageIO.cxx
  itkMGHImageIOFactory.cxx
  )

set(include_dirs
  ${MGHImageIO_SOURCE_DIR}
  ${MGHImageIO_BINARY_DIR}
  )

#if(EXISTS "${ITK_DIR}/CMakeCache.txt")
#  set(include_dirs ${include_dirs} "${ITK_DIR}/Utilities/zlib")
#else(EXISTS "${ITK_DIR}/CMakeCache.txt")
#endif(EXISTS "${ITK_DIR}/CMakeCache.txt")

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

add_library(MGHIO ${MGHImageIO_SRCS})
target_link_libraries(MGHIO ITKIO MRML)

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} MGHIO)

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# MRMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).
add_library(MGHIOPlugin SHARED itkMGHIOPlugin.cxx)
target_link_libraries(MGHIOPlugin MGHIO)
# We should switch to SET_TARGET_PROPERTIES() with LIBRARY_OUTPUT_DIRECTORY 
# once CMake 2.6 is released.  Until then, we need to hack a solution with
# ADD_CUSTOM_COMMAND()
#SET_TARGET_PROPERTIES(MGHIOPlugin PROPERTIES 
#              LIBRARY_OUTPUT_DIRECTORY "${SLICER_LIBRARIES_DIR}/ITKFactories")
get_target_property(LIBFULLNAME MGHIOPlugin LOCATION)
get_filename_component(LIBNAME ${LIBFULLNAME} NAME)

add_custom_command(TARGET MGHIOPlugin
  DEPENDS ${LIB_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBFULLNAME} ${SLICER_LIBRARIES_DIR}/ITKFactories/${CMAKE_CFG_INTDIR}/${LIBNAME})

# make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs
if(APPLE)
  add_custom_command(
    TARGET MGHIOPlugin
    DEPENDS ${Slicer3_BINARY_DIR}/bin/libMGHIOPlugin.dylib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${SLICER_LIBRARIES_DIR}/ITKFactories/libMGHIOPlugin.dylib
    ${SLICER_LIBRARIES_DIR}/ITKFactories/libMGHIOPlugin.so
    )
endif(APPLE)

if(BUILD_SHARED_LIBS)
  # Note MGHIO and MGHIOPlugin are installed in different locations
  install(TARGETS MGHIO 
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
  install(TARGETS MGHIOPlugin
    RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/ITKFactories COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

configure_file(
  ${MGHImageIO_SOURCE_DIR}/itkMGHImageIOConfigure.h.in 
  ${MGHImageIO_BINARY_DIR}/itkMGHImageIOConfigure.h
  )
