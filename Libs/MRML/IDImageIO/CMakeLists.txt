project(MRMLIDImageIO)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.13.4)
#-----------------------------------------------------------------------------


# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------
if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# ITK
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ITKCommon
  ITKIOImageBase
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS ${ITKFactoryRegistration_INCLUDE_DIRS})
include(${ITK_USE_FILE})

#
# VTK
#
#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRMLCore_INCLUDE_DIRS}
  )
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file itkMRMLIDImageIOConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(MRMLIDImageIO_SRCS
  itkMRMLIDImageIO.cxx
  itkMRMLIDImageIOFactory.cxx
  )

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
# Note: Library name is different from the directory name !
set(lib_name MRMLIDIO)

set(srcs ${MRMLIDImageIO_SRCS})
add_library(${lib_name} ${srcs})

set(libs MRMLCore)
target_link_libraries(${lib_name} ${libs})

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Folder
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_FOLDER)
  set(${PROJECT_NAME}_FOLDER ${PROJECT_NAME})
endif()
if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
  set_target_properties(${lib_name} PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# MRMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).

if(NOT DEFINED MRMLIDImageIO_ITKFACTORIES_DIR)
  set(MRMLIDImageIO_ITKFACTORIES_DIR lib/ITKFactories)
endif()
if(NOT DEFINED MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR)
  set(MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR ${MRMLIDImageIO_ITKFACTORIES_DIR})
endif()

add_library(MRMLIDIOPlugin SHARED
  itkMRMLIDIOPlugin.cxx
  )

set_target_properties(MRMLIDIOPlugin PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${MRMLIDImageIO_ITKFACTORIES_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${MRMLIDImageIO_ITKFACTORIES_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${MRMLIDImageIO_ITKFACTORIES_DIR}"
  )
target_link_libraries(MRMLIDIOPlugin ${lib_name})

# Folder
if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
  set_target_properties(MRMLIDIOPlugin PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
endif()

# --------------------------------------------------------------------------
# Install library - MRMLIDIO and MRMLIDOPlugin are installed in different locations
# --------------------------------------------------------------------------
install(TARGETS MRMLIDIOPlugin
  RUNTIME DESTINATION ${MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
