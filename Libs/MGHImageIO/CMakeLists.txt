project(MGHImageIO)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

#if(EXISTS "${ITK_DIR}/CMakeCache.txt")
#  set(include_dirs ${include_dirs} "${ITK_DIR}/Utilities/zlib")
#else(EXISTS "${ITK_DIR}/CMakeCache.txt")
#endif(EXISTS "${ITK_DIR}/CMakeCache.txt")

include_directories(${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/itkMGHImageIOConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/itkMGHImageIOConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/itkMGHImageIOConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(MGHImageIO_SRCS
  itkMGHImageIO.cxx
  itkMGHImageIOFactory.cxx
  )

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name MGHIO)
add_library(${lib_name} 
  ${MGHImageIO_SRCS}
  )

target_link_libraries(${lib_name} 
  ITKIO 
)

install(TARGETS ${lib_name} 
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# MRMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).

add_library(MGHIOPlugin SHARED 
  itkMGHIOPlugin.cxx
  )
target_link_libraries(MGHIOPlugin 
  ${lib_name}
)

if(NOT DEFINED MGHImageIO_INSTALL_ITKFACTORIES_DIR)
  set(MGHImageIO_INSTALL_ITKFACTORIES_DIR lib/ITKFactories)
endif(NOT DEFINED MGHImageIO_INSTALL_ITKFACTORIES_DIR)

# We should switch to SET_TARGET_PROPERTIES() with LIBRARY_OUTPUT_DIRECTORY 
# once CMake 2.6 is released.  Until then, we need to hack a solution with
# ADD_CUSTOM_COMMAND()
#SET_TARGET_PROPERTIES(MGHIOPlugin PROPERTIES 
#              LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${MGHImageIO_INSTALL_ITKFACTORIES_DIR}")

get_target_property(LIBFULLNAME MGHIOPlugin LOCATION)
get_filename_component(LIBNAME ${LIBFULLNAME} NAME)

add_custom_command(TARGET MGHIOPlugin
  DEPENDS ${LIB_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBFULLNAME} ${CMAKE_BINARY_DIR}/${MGHImageIO_INSTALL_ITKFACTORIES_DIR}/${CMAKE_CFG_INTDIR}/${LIBNAME})

# make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs

if(APPLE)
  add_custom_command(
    TARGET MGHIOPlugin
    DEPENDS ${MGHIO_BINARY_DIR}/bin/libMGHIOPlugin.dylib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/${MGHImageIO_INSTALL_ITKFACTORIES_DIR}/libMGHIOPlugin.dylib
    ${CMAKE_BINARY_DIR}/${MGHImageIO_INSTALL_ITKFACTORIES_DIR}/libMGHIOPlugin.so
    )
endif(APPLE)

install(TARGETS MGHIOPlugin
  RUNTIME DESTINATION ${MGHImageIO_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${MGHImageIO_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )
