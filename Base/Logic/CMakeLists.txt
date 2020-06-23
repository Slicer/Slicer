project(SlicerBaseLogic)

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# ModuleDescriptionParser
#
if(Slicer_BUILD_CLI_SUPPORT)
  find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)
endif()

#
# VTK
#
if(Slicer_USE_PYTHONQT AND NOT VTK_WRAP_PYTHON)
  message(FATAL_ERROR "Since Slicer_USE_PYTHONQT is ON, VTK_WRAP_PYTHON is expected to be ON. "
                      "Re-configure VTK with python wrapping.")
endif()

#
# ITK
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ITKCommon
  ITKIOTransformBase
  ITKTransform
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND ${PROJECT_NAME}_ITK_COMPONENTS ${ModuleDescriptionParser_ITK_COMPONENTS})
endif()
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS
  ${ITKFactoryRegistration_INCLUDE_DIRS}
  )
include(${ITK_USE_FILE})

#
# LibArchive
#
find_package(LibArchive REQUIRED MODULE)

# --------------------------------------------------------------------------
# Subdirectories
# --------------------------------------------------------------------------
add_subdirectory(Resources)

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRMLCore_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  ${vtkTeem_INCLUDE_DIRS}
  ${RemoteIO_INCLUDE_DIRS}
  ${LibArchive_INCLUDE_DIR}
  )

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND include_dirs
    ${MRMLCLI_INCLUDE_DIRS}
    ${ModuleDescriptionParser_INCLUDE_DIRS}
    )
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND include_dirs
    ${PYTHON_INCLUDE_DIR}
    )
endif()

include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Update Slicer_Base_INCLUDE_DIRS
# --------------------------------------------------------------------------
set(Slicer_Base_INCLUDE_DIRS ${Slicer_Base_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "Slicer Base includes" FORCE)

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkSlicerBaseLogicConfigure.h)
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
set(SlicerBaseLogic_SRCS
  # Logic classes (data management and calculation)
  vtkSlicerApplicationLogic.cxx
  vtkSlicerModuleLogic.cxx
  vtkSlicerTask.cxx
  vtkSlicerFiducialsLogic.cxx
  vtkDataIOManagerLogic.cxx
  # slicer's vtk extensions (filters)
  vtkSlicerGlyphSource2D.cxx
  vtkTransformVisualizerGlyph3D.cxx
  vtkImageRectangularSource.cxx
  vtkSystemInformation.cxx
  vtkImageFillROI.cxx
  )

if(Slicer_USE_PYTHONQT)
  list(APPEND SlicerBaseLogic_SRCS
    vtkSlicerScriptedLoadableModuleLogic.cxx
    )
endif()

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name SlicerBaseLogic)

#set(srcs ${SlicerBaseLogic_SRCS} ${Instantiator_SRCS})
set(srcs ${SlicerBaseLogic_SRCS})

add_library(${lib_name} ${srcs})

set(libs
  MRMLLogic
  ${VTK_LIBRARIES}
  )

# Archive support used by extensions and by slicer data bundle
list(APPEND libs
  ${LibArchive_LIBRARY}
  )

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND libs
    MRMLCLI
    )
endif()

if(Slicer_USE_PYTHONQT)
  set(VTK_COMMON_PYTHON_LIBRARY
    vtkWrappingPythonCore # For vtkPythonUtil
    )
  list(APPEND libs
    ${VTK_COMMON_PYTHON_LIBRARY}
    ${PYTHON_LIBRARY} # At some point, VTK should be updated to expose VTK_PYTHON_LIBRARY
    )
endif()

target_link_libraries(${lib_name} ${libs})
set_target_properties(${lib_name} PROPERTIES LABELS ${lib_name})
set_target_properties(${lib_name} PROPERTIES FOLDER "Core-Base")

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS ${lib_name})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
if(VTK_WRAP_PYTHON)
  include(vtkMacroKitPythonWrap)

  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${SlicerBaseLogic_SRCS}
    KIT_PYTHON_LIBRARIES MRMLLogicPythonD
    KIT_INSTALL_BIN_DIR ${Slicer_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR}
    )
  # Export target
  set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS ${lib_name}Python ${lib_name}PythonD)

  set_target_properties(${lib_name}Python PROPERTIES FOLDER "Core-Base")
  set_target_properties(${lib_name}PythonD PROPERTIES FOLDER "Core-Base")
  if(TARGET ${lib_name}Hierarchy)
    set_target_properties(${lib_name}Hierarchy PROPERTIES FOLDER "Core-Base")
  endif()
endif()

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

