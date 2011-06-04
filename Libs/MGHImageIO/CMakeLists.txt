project(MGHImageIO)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
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
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  SET(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
ENDIF()
IF(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
ENDIF()

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
IF(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  SET(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
ENDIF()
EXPORT(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  SET(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
ENDIF()
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  SET(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
ENDIF()

INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# MRMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).

if(NOT DEFINED MGHImageIO_INSTALL_ITKFACTORIES_DIR)
  set(MGHImageIO_INSTALL_ITKFACTORIES_DIR lib/ITKFactories)
endif(NOT DEFINED MGHImageIO_INSTALL_ITKFACTORIES_DIR)

add_library(MGHIOPlugin SHARED 
  itkMGHIOPlugin.cxx
  )

set_target_properties(MGHIOPlugin
  PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY 
  "${CMAKE_BINARY_DIR}/${Slicer_ITKFACTORIES_DIR}"
  LIBRARY_OUTPUT_DIRECTORY 
  "${CMAKE_BINARY_DIR}/${Slicer_ITKFACTORIES_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY 
  "${CMAKE_BINARY_DIR}/${Slicer_ITKFACTORIES_DIR}"
  )
target_link_libraries(MGHIOPlugin 
  ${lib_name}
)

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF()

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
    DEPENDS "${CMAKE_BINARY_DIR}/${Slicer_ITKFACTORIES_DIR}/libMGHIOPlugin.dylib"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_BINARY_DIR}/${Slicer_ITKFACTORIES_DIR}/libMGHIOPlugin.dylib"
    "${CMAKE_BINARY_DIR}/${Slicer_ITKFACTORIES_DIR}/libMGHIOPlugin.so"
    VERBATIM
    )
  install(FILES
    "${CMAKE_BINARY_DIR}/${Slicer_ITKFACTORIES_DIR}/libMGHIOPlugin.so"
    DESTINATION ${MGHImageIO_INSTALL_ITKFACTORIES_DIR}
    )
endif()
