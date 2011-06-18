project(IGT)

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

#find_package(KWWidgets REQUIRED)
#include(${KWWidgets_USE_FILE})
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()
 
# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MRML_INCLUDE_DIRS}
  )
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkIGTConfigure.h)
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
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.(h|txx)")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
ENDIF()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(IGT_SRCS
  vtkIGTDataManager.cxx
  vtkIGTDataStream.cxx
  vtkIGTMatrixState.cxx
  vtkIGTPat2ImgRegistration.cxx
  )
set(TRACKING_INTERFACE_LIBRARY "")

#--- NH took out the following section in an effot to remove Navitrak
#--- 10/16/07

#IF(IGT_USE_NAVITRACK)
#  SET(IGT_SRCS
#    vtkIGTDataManager.cxx
#    vtkIGTDataStream.cxx
#    vtkIGTMatrixState.cxx
#    vtkIGTPat2ImgRegistration.cxx
#    vtkIGTOpenTrackerStream.cxx
#  )
#  SET(TRACKING_INTERFACE_INCLUDE_DIR
#    ${NAVITRACK_INC}
#  )
#  SET(TRACKING_INTERFACE_LIB  
#    ${NAVITRACK_LIB}
#  )
#ENDIF()

if(IGT_USE_IGSTK)
  set(IGT_SRCS
    vtkIGTDataManager.cxx
    vtkIGTDataStream.cxx
    vtkIGTMatrixState.cxx
    vtkIGTPat2ImgRegistration.cxx
    vtkIGTIGSTKStream.cxx
    )
  set(TRACKING_INTERFACE_INCLUDE_DIR
    ${IGSTK_INCLUDE_DIRS}
    )
  set(TRACKING_INTERFACE_LIB  
    ${IGSTK_LIBRARIES}
    )
endif()

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})

SET(srcs ${IGT_SRCS})
add_library(${lib_name} ${srcs})

SET(libs
  MRML
  ${TRACKING_INTERFACE_LIB}
)

# If VTK_WRAP_TCL: Complain if KWWidget is not available
#  IF(NOT KWWidgets_FOUND)
#    MESSAGE(FATAL_ERROR "IGT required KWWidgets to build")
#  ELSE(NOT KWWidgets_FOUND)
#    ADD_DEFINITIONS(-DKWWidgets_FOUND)
#  ENDIF(NOT KWWidgets_FOUND)

target_link_libraries(${lib_name} ${libs})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF()

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

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
SET(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
