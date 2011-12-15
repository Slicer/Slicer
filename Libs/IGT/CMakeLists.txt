project(IGT)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.4)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
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
  ${MRMLCore_INCLUDE_DIRS}
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
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.(h|txx)")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

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

#if(IGT_USE_NAVITRACK)
#  set(IGT_SRCS
#    vtkIGTDataManager.cxx
#    vtkIGTDataStream.cxx
#    vtkIGTMatrixState.cxx
#    vtkIGTPat2ImgRegistration.cxx
#    vtkIGTOpenTrackerStream.cxx
#  )
#  set(TRACKING_INTERFACE_INCLUDE_DIR
#    ${NAVITRACK_INC}
#  )
#  set(TRACKING_INTERFACE_LIB
#    ${NAVITRACK_LIB}
#  )
#endif()

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

set(srcs ${IGT_SRCS})
add_library(${lib_name} ${srcs})

set(libs
  MRMLCore
  ${TRACKING_INTERFACE_LIB}
)

# If VTK_WRAP_TCL: Complain if KWWidget is not available
#  if(NOT KWWidgets_FOUND)
#    message(FATAL_ERROR "IGT required KWWidgets to build")
#  else(NOT KWWidgets_FOUND)
#    add_definitions(-DKWWidgets_FOUND)
#  endif(NOT KWWidgets_FOUND)

target_link_libraries(${lib_name} ${libs})

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
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
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
