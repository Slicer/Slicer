project(RemoteIO)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.13.4)
#-----------------------------------------------------------------------------

# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------

# Disable MSVC 8 warnings
if(WIN32)
  option(DISABLE_MSVC8_DEPRECATED_WARNINGS
    "Disable Visual Studio 8 deprecated warnings" ON)
  mark_as_advanced(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)
  if(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif()
endif()

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# VTK
#
#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

#
# CURL
#
find_package(CURL REQUIRED)

#
# OpenSSL
#
if(Slicer_USE_PYTHONQT_WITH_OPENSSL)
  find_package(OpenSSL REQUIRED)
endif()

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRMLCore_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  )
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkRemoteIOConfigure.h)
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
set(RemoteIO_SRCS
  vtkHTTPHandler.cxx
  )

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})
set(srcs ${RemoteIO_SRCS})

add_library(${lib_name} ${srcs})

set(libs
  ${CURL_LIBRARIES}
  MRMLCore
  )
if(Slicer_USE_PYTHONQT_WITH_OPENSSL)
  list(APPEND libs
    ${OPENSSL_LIBRARIES}
    )
endif()
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

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
