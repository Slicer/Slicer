project(SlicerBaseCLI)

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# ModuleDescriptionParser
#
find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)


#
# ITK - Import ITK targets required by ModuleDescriptionParser
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ${ModuleDescriptionParser_ITK_COMPONENTS}
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(Slicer_BaseCLI_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Testing
  )
set(include_dirs
  ${Slicer_BaseCLI_INCLUDE_DIRS}
  ${ModuleDescriptionParser_INCLUDE_DIRS}
  )
include_directories(${include_dirs})
  
# --------------------------------------------------------------------------
# Update SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES
# --------------------------------------------------------------------------
set(SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES
  ${SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES}
  ${Slicer_BaseCLI_INCLUDE_DIRS}
  ${ITKFactoryRegistration_INCLUDE_DIRS}
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
  )
set(SlicerBaseCLI_LIBS
  ModuleDescriptionParser ${ITK_LIBRARIES}
  )

#find_package(VTK)
if(VTK_FOUND)
  #include(${VTK_USE_FILE})
  list(APPEND SlicerBaseCLI_SRCS vtkPluginFilterWatcher.cxx)
  list(APPEND SlicerBaseCLI_LIBS ${VTK_LIBRARIES})
endif()

if(SlicerBaseCLI_SRCS)
  # --------------------------------------------------------------------------
  # Build library
  # --------------------------------------------------------------------------
  set(lib_name "SlicerBaseCLI")

  add_library(${lib_name} ${SlicerBaseCLI_SRCS})
  target_link_libraries(${lib_name} ${SlicerBaseCLI_LIBS})
  set_target_properties(${lib_name} PROPERTIES LABELS ${lib_name})
  set_target_properties(${lib_name} PROPERTIES FOLDER "Core-Base")

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
endif()
