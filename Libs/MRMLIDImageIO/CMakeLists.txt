project(MRMLIDImageIO)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/itkMRMLIDImageIOConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/itkMRMLIDImageIOConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} "${CMAKE_CURRENT_BINARY_DIR}/itkMRMLIDImageIOConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(MRMLIDImageIO_SRCS
  itkMRMLIDImageIO.cxx
  itkMRMLIDImageIOFactory.cxx
  )

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name MRMLIDIO)
add_library(${lib_name} 
  ${MRMLIDImageIO_SRCS}
  )
target_link_libraries(${lib_name} 
  ITKIO 
  MRML
)

slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES ${tmp} ${lib_name})

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

add_library(MRMLIDIOPlugin SHARED itkMRMLIDIOPlugin.cxx)
target_link_libraries(MRMLIDIOPlugin ${lib_name})

# We should switch to SET_TARGET_PROPERTIES() with LIBRARY_OUTPUT_DIRECTORY 
# once CMake 2.6 is released.  Until then, we need to hack a solution with
# ADD_CUSTOM_COMMAND()
#SET_TARGET_PROPERTIES(MRMLIDIOPlugin PROPERTIES 
#              LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_ITKFACTORIES_DIR}")

get_target_property(LIBFULLNAME MRMLIDIOPlugin LOCATION)
get_filename_component(LIBNAME ${LIBFULLNAME} NAME)

add_custom_command(TARGET MRMLIDIOPlugin
  DEPENDS ${LIB_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${LIBFULLNAME} ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_ITKFACTORIES_DIR}/${CMAKE_CFG_INTDIR}/${LIBNAME})

# Note MRMLIDIO and MRMLIDOPlugin are installed in different locations
install(TARGETS MRMLIDIOPlugin
  RUNTIME DESTINATION ${Slicer3_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer3_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs
if(APPLE)
  add_custom_command(
    TARGET MRMLIDIOPlugin
    DEPENDS ${Slicer3_BINARY_DIR}/bin/libMRMLIDIOPlugin.dylib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_ITKFACTORIES_DIR}/libMRMLIDIOPlugin.dylib
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_ITKFACTORIES_DIR}/libMRMLIDIOPlugin.so
    )
endif(APPLE)
