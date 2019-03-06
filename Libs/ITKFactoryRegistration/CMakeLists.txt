project(ITKFactoryRegistration)

cmake_minimum_required(VERSION 3.13.4)

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# ITK
#
set(ITK_IO_MODULES_USED
  ITKIOJPEG
  ITKIOGDCM
  ITKIOBMP
  ITKIOLSM
  ITKIOPNG
  ITKIOTIFF
  ITKIOVTK
  ITKIOStimulate
  ITKIOBioRad
  ITKIOMeta
  ITKIOMRC
  ITKIONIFTI
  ITKIONRRD
  ITKIOGIPL
  ITKIOTransformHDF5
  ITKIOTransformInsightLegacy
  ITKIOTransformMatlab
  MGHIO
  ITKIOMINC
  )
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND ITK_IO_MODULES_USED ITKIODCMTK)
endif()

set(${PROJECT_NAME}_ITK_COMPONENTS
  ITKIOImageBase
  ITKIOTransformBase
  ${ITK_IO_MODULES_USED}
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)

# This is the Slicer library in charge of registering the default ITK factories.
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 0)

include(${ITK_USE_FILE})

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
set(configure_header_file itkFactoryRegistrationConfigure.h)
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
set(srcs
  itkFactoryRegistration.cxx
  )

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})

add_library(${lib_name} SHARED ${srcs})

set(libs
  ${ITK_LIBRARIES}
  )
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
