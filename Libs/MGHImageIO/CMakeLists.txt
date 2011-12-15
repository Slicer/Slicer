project(MGHImageIO)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.4)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file itkMGHImageIOConfigure.h)
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
set(MGHImageIO_SRCS
  itkMGHImageIO.cxx
  itkMGHImageIOFactory.cxx
  )

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
# Note: Library name is different from the directory name !
set(lib_name MGHIO)

set(srcs ${MGHImageIO_SRCS})
add_library(${lib_name} ${srcs})

set(libs ${ITK_LIBRARIES})
target_link_libraries(${lib_name} ${libs})

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

if(NOT DEFINED MGHImageIO_ITKFACTORIES_DIR)
  set(MGHImageIO_ITKFACTORIES_DIR lib/ITKFactories)
endif()
if(NOT DEFINED MGHImageIO_INSTALL_ITKFACTORIES_DIR)
  set(MGHImageIO_INSTALL_ITKFACTORIES_DIR ${MGHImageIO_ITKFACTORIES_DIR})
endif()

add_library(MGHIOPlugin SHARED
  itkMGHIOPlugin.cxx
  )

set_target_properties(MGHIOPlugin PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${MGHImageIO_ITKFACTORIES_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${MGHImageIO_ITKFACTORIES_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${MGHImageIO_ITKFACTORIES_DIR}"
  )
target_link_libraries(MGHIOPlugin
  ${lib_name}
)

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Install library - MGHIO and MGHIOPlugin are installed in different locations
# --------------------------------------------------------------------------
install(TARGETS MGHIOPlugin
  RUNTIME DESTINATION ${MGHImageIO_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${MGHImageIO_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Make a copy named '.so' for compatibility with the expectations
# of the ITK Factory mechanism on macs
# --------------------------------------------------------------------------
if(APPLE)
  add_custom_command(
    TARGET MGHIOPlugin
    DEPENDS "${CMAKE_BINARY_DIR}/${MGHImageIO_ITKFACTORIES_DIR}/libMGHIOPlugin.dylib"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_BINARY_DIR}/${MGHImageIO_ITKFACTORIES_DIR}/libMGHIOPlugin.dylib"
    "${CMAKE_BINARY_DIR}/${MGHImageIO_ITKFACTORIES_DIR}/libMGHIOPlugin.so"
    VERBATIM
    )
  install(FILES
    "${CMAKE_BINARY_DIR}/${MGHImageIO_ITKFACTORIES_DIR}/libMGHIOPlugin.so"
    DESTINATION ${MGHImageIO_INSTALL_ITKFACTORIES_DIR}
    )
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
