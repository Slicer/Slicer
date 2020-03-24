project(vtkSegmentationCore)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.13.4)
#-----------------------------------------------------------------------------

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkSegmentationCoreConfigure.h)
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

set(vtkSegmentationCore_SRCS
  vtkOrientedImageData.cxx
  vtkOrientedImageData.h
  vtkOrientedImageDataResample.cxx
  vtkOrientedImageDataResample.h
  vtkSegment.cxx
  vtkSegment.h
  vtkSegmentation.cxx
  vtkSegmentation.h
  vtkSegmentationConverter.cxx
  vtkSegmentationConverter.h
  vtkSegmentationConverterFactory.cxx
  vtkSegmentationConverterFactory.h
  vtkSegmentationConverterRule.cxx
  vtkSegmentationConverterRule.h
  vtkSegmentationHistory.cxx
  vtkSegmentationHistory.h
  vtkSegmentationModifier.cxx
  vtkSegmentationModifier.h
  vtkTopologicalHierarchy.cxx
  vtkTopologicalHierarchy.h
  vtkBinaryLabelmapToClosedSurfaceConversionRule.cxx
  vtkBinaryLabelmapToClosedSurfaceConversionRule.h
  vtkClosedSurfaceToBinaryLabelmapConversionRule.cxx
  vtkClosedSurfaceToBinaryLabelmapConversionRule.h
  vtkCalculateOversamplingFactor.cxx
  vtkCalculateOversamplingFactor.h
  vtkClosedSurfaceToFractionalLabelmapConversionRule.h
  vtkClosedSurfaceToFractionalLabelmapConversionRule.cxx
  vtkFractionalLabelmapToClosedSurfaceConversionRule.h
  vtkFractionalLabelmapToClosedSurfaceConversionRule.cxx
  vtkPolyDataToFractionalLabelmapFilter.h
  vtkPolyDataToFractionalLabelmapFilter.cxx
  )

# Abstract/pure virtual classes

#set_source_files_properties(
#  ABSTRACT
#  )

#
# vtkAddon
#
if(NOT Slicer_BUILD_vtkAddon)
  find_package(vtkAddon REQUIRED)
endif()

set(vtkSegmentationCore_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${vtkAddon_INCLUDE_DIRS}
  CACHE INTERNAL "" FORCE
  )

# --------------------------------------------------------------------------
# Build the library
# --------------------------------------------------------------------------

set(vtkSegmentationCore_LIBS
  ${VTK_LIBRARIES}
  )

include_directories( ${vtkSegmentationCore_INCLUDE_DIRS} )
add_library(${PROJECT_NAME} ${vtkSegmentationCore_SRCS})
target_link_libraries( ${PROJECT_NAME} ${vtkSegmentationCore_LIBS} )

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${PROJECT_NAME} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Folder
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_FOLDER)
  set(${PROJECT_NAME}_FOLDER ${PROJECT_NAME})
endif()
if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
  set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${PROJECT_NAME} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python wrapping
# --------------------------------------------------------------------------
if(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

  set(Slicer_Wrapped_LIBRARIES
    )

  foreach(library ${PROJECT_NAME})
    if(TARGET ${library}PythonD)
      list(APPEND Slicer_Wrapped_LIBRARIES ${library}PythonD)
    endif()
  endforeach()

  include(vtkMacroKitPythonWrap)

  vtkMacroKitPythonWrap(
    KIT_NAME ${PROJECT_NAME}
    KIT_SRCS ${vtkSegmentationCore_SRCS}
    KIT_INSTALL_BIN_DIR ${${PROJECT_NAME}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${PROJECT_NAME}_INSTALL_LIB_DIR}
    )

  # Export target
  export(TARGETS ${PROJECT_NAME}Python ${PROJECT_NAME}PythonD APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})
  # Folder
  if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
    set_target_properties(${PROJECT_NAME}Python PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    set_target_properties(${PROJECT_NAME}PythonD PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    if(TARGET ${PROJECT_NAME}Hierarchy)
      set_target_properties(${PROJECT_NAME}Hierarchy PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    endif()
  endif()
endif()

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)

