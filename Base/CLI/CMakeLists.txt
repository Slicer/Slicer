project(SlicerBaseCLI)

# --------------------------------------------------------------------------
# Third party library
# --------------------------------------------------------------------------
find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(Slicer_BaseCLI_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  )
if(ITK_VERSION_MAJOR GREATER 3)
  list(APPEND Slicer_BaseCLI_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Testing)
endif()
set(include_dirs
  ${Slicer_BaseCLI_INCLUDE_DIRS}
  ${ModuleDescriptionParser_INCLUDE_DIRS}
  )
include_directories(${include_dirs})
  
# --------------------------------------------------------------------------
# Update SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES
# --------------------------------------------------------------------------
set(SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES
  ${SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES} ${Slicer_BaseCLI_INCLUDE_DIRS}
  CACHE INTERNAL "SlicerExecutionModel extra includes" FORCE
  )
if(${ITK_VERSION_MAJOR} GREATER 3)
  set(SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES
    ${SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES} ${ITKFactoryRegistration_INCLUDE_DIRS}
    CACHE INTERNAL "SlicerExecutionModel extra includes" FORCE
    )
  set(SlicerExecutionModel_CLI_LIBRARY_WRAPPER_CXX
    ${CMAKE_CURRENT_BINARY_DIR}/SEMCommandLineLibraryWrapper.cxx
    CACHE INTERNAL "SlicerExecutionModel extra includes" FORCE
    )
  configure_file(
    SEMCommandLineLibraryWrapper.cxx.in
    ${SlicerExecutionModel_CLI_LIBRARY_WRAPPER_CXX}
    )
endif()

# --------------------------------------------------------------------------
# Update Slicer_Base_INCLUDE_DIRS
# --------------------------------------------------------------------------
set(Slicer_Base_INCLUDE_DIRS ${Slicer_Base_INCLUDE_DIRS}
  ${Slicer_BaseCLI_INCLUDE_DIRS} CACHE INTERNAL "Slicer Base includes" FORCE)

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkSlicerBaseCLIConfigure.h)
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
# Single library target. Down the road, we may want to have one library that
# only depends on ITK and a second library that only depends on VTK

set(SlicerBaseCLI_SRCS
  vtkPluginFilterWatcher.cxx
  )

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name "SlicerBaseCLI")

add_library(${lib_name} ${SlicerBaseCLI_SRCS})
target_link_libraries(${lib_name}
  ${VTK_LIBRARIES}
  ModuleDescriptionParser
  )
set_target_properties(${lib_name} PROPERTIES LABELS ${lib_name})

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

