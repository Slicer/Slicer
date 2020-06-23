project(MRMLDisplayableManager)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.13.4)
#-----------------------------------------------------------------------------

# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------
if(Slicer_USE_PYTHONQT AND VTK_WRAP_PYTHON)
  set(MRMLDisplayableManager_USE_PYTHON ON)
else()
  set(MRMLDisplayableManager_USE_PYTHON OFF)
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
if(Slicer_USE_PYTHONQT AND NOT VTK_WRAP_PYTHON)
  message(FATAL_ERROR "Since Slicer_USE_PYTHONQT is ON, VTK_WRAP_PYTHON is expected to be ON. "
                      "Re-configure VTK with python wrapping.")
endif()

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRMLCore_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  )
if(MRMLDisplayableManager_USE_PYTHON)
  list(APPEND include_dirs ${PYTHON_INCLUDE_DIR})
endif()
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkMRMLDisplayableManagerConfigure.h)
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
set(DisplayableManager_SRCS

  # DisplayableManager associated with ThreeDView
  vtkMRMLCameraDisplayableManager.cxx
  vtkMRMLModelDisplayableManager.cxx
  vtkMRMLViewDisplayableManager.cxx
  vtkMRMLThreeDReformatDisplayableManager.cxx

  # DisplayableManager associated with SliceView
  vtkMRMLCrosshairDisplayableManager.cxx
  vtkMRMLCrosshairDisplayableManager3D.cxx
  vtkMRMLModelSliceDisplayableManager.cxx
  vtkMRMLVolumeGlyphSliceDisplayableManager.cxx

  # DisplayableManager common between ThreeDView and SliceView
  vtkMRMLOrientationMarkerDisplayableManager.cxx
  vtkMRMLRulerDisplayableManager.cxx
  vtkMRMLScalarBarDisplayableManager.cxx
  )

set(KIT_SRCS
  ${DisplayableManager_SRCS}
  vtkMRMLAbstractDisplayableManager.cxx
  vtkMRMLDisplayableManagerGroup.cxx
  vtkMRMLDisplayableManagerFactory.cxx

  vtkMRMLInteractionEventData.cxx

  # ThreeDView factory and DisplayableManager
  vtkMRMLAbstractThreeDViewDisplayableManager.cxx
  vtkMRMLThreeDViewDisplayableManagerFactory.cxx

  # SliceView factory and DisplayableManager
  vtkMRMLAbstractSliceViewDisplayableManager.cxx
  vtkMRMLSliceViewDisplayableManagerFactory.cxx

  vtkMRMLViewInteractorStyle.cxx
  vtkMRMLSliceViewInteractorStyle.cxx
  vtkMRMLThreeDViewInteractorStyle.cxx

  # Widgets
  vtkMRMLAbstractWidget.cxx
  vtkMRMLAbstractWidgetRepresentation.cxx
  vtkMRMLCameraWidget.cxx
  vtkMRMLSliceIntersectionWidget.cxx
  vtkMRMLSliceIntersectionRepresentation2D.cxx
  vtkMRMLRubberBandWidgetRepresentation.cxx
  vtkMRMLWindowLevelWidget.cxx

  # Proxy classes
  vtkMRMLLightBoxRendererManagerProxy.cxx
  )

set_source_files_properties(
  vtkMRMLDisplayableManagerGroup.cxx
  WRAP_EXCLUDE
  )

if(MRMLDisplayableManager_USE_PYTHON)
  list(APPEND KIT_SRCS
    vtkMRMLScriptedDisplayableManager.cxx
    )
  set_source_files_properties(
    vtkMRMLScriptedDisplayableManager.cxx
    WRAP_EXCLUDE
    )
endif()

# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})

# --------------------------------------------------------------------------
# Create the instantiator for DisplayableManager classes.
# --------------------------------------------------------------------------
include(SlicerConfigureDisplayableManagerObjectFactory)
SlicerConfigureDisplayableManagerObjectFactory(
  TARGET_NAME ${lib_name}
  SRCS "${DisplayableManager_SRCS}"
  EXPORT_MACRO "VTK_MRML_DISPLAYABLEMANAGER_EXPORT"
  EXPORT_HEADER "vtk${PROJECT_NAME}Export.h"
  OUTPUT_SRCS_VAR DisplayableManagerInstantiator_SRCS
  )
set(srcs
  ${KIT_SRCS}
  ${DisplayableManagerInstantiator_SRCS}
  )

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
add_library(${lib_name} ${srcs})

set(libs
  ${VTK_LIBRARIES}
  MRMLCore
  MRMLLogic
  )

if(MRMLDisplayableManager_USE_PYTHON)
  set(VTK_COMMON_PYTHON_LIBRARY
    vtkWrappingPythonCore # For vtkPythonUtil
    )
  list(APPEND libs
    ${VTK_COMMON_PYTHON_LIBRARY} # For vtkPythonUtils
    ${PYTHON_LIBRARY} # At some point, VTK should be updated to expose VTK_PYTHON_LIBRARY
    )
endif()

target_link_libraries(${lib_name} ${libs})

if(${lib_name}_AUTOINIT)
  set_property(TARGET ${lib_name}
    APPEND PROPERTY COMPILE_DEFINITIONS
    "${lib_name}_AUTOINIT=1(${lib_name})"
    )
endif()

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
# Python Wrapping
# --------------------------------------------------------------------------
if(MRMLDisplayableManager_USE_PYTHON)
  include(vtkMacroKitPythonWrap)

  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${KIT_SRCS}
    KIT_PYTHON_LIBRARIES MRMLLogicPythonD
    KIT_INSTALL_BIN_DIR ${${PROJECT_NAME}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${PROJECT_NAME}_INSTALL_LIB_DIR}
    )
  # Export target
  export(TARGETS ${lib_name}Python ${lib_name}PythonD APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})
  # Folder
  if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
    set_target_properties(${lib_name}Python PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    set_target_properties(${lib_name}PythonD PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    if(TARGET ${lib_name}Hierarchy)
      set_target_properties(${lib_name}Hierarchy PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    endif()
  endif()

  add_subdirectory(Python)
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
