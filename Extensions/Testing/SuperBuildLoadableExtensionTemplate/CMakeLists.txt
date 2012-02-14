#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.4)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

#-----------------------------------------------------------------------------
# Module name
#   - Usually identique to the directory name)
#   - You shouldn't call 'project(...)'. See SlicerConfig.cmake for more details.
#-----------------------------------------------------------------------------
set(EXTENSION_NAME SuperBuildLoadableExtensionTemplate)
set(EXTENSION_CATEGORY "Examples")
set(EXTENSION_BUILD_SUBDIRECTORY ${EXTENSION_NAME}-build)

#-----------------------------------------------------------------------------
# Prerequisites
#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)

  # Additional C/CXX flags - Should be defined before including Slicer_USE_FILE
  set(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} C Flags")
  set(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} CXX Flags")

  include(${Slicer_USE_FILE})

  include(SlicerEnableExtensionTesting)

  find_package(Git REQUIRED)
endif()

#-----------------------------------------------------------------------------
# Superbuild
#-----------------------------------------------------------------------------
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)
if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#-----------------------------------------------------------------------------
# Add subdirectories
#-----------------------------------------------------------------------------
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
# Extension description
#-----------------------------------------------------------------------------
set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting")
set(EXTENSION_STATUS "Beta")
set(EXTENSION_DESCRIPTION "This is an example of Qt loadable module using SuperBuild built as an extension")
set(EXTENSION_DEPENDS CLIExtensionTemplate) # Specified as a space separated list or 'NA' if any

set(EXTENSION_LICENSE_FILE ${Slicer_LICENSE_FILE})
set(EXTENSION_README_FILE ${Slicer_README_FILE})

#-----------------------------------------------------------------------------
# Sources
#-----------------------------------------------------------------------------
set(qt_module_export_directive "Q_SLICER_QTMODULES_SUPERBUILDLOADABLEEXTENSIONTEMPLATE_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

# Source files
set(qt_module_SRCS
  qSlicerSuperBuildLoadableExtensionTemplateModule.cxx
  qSlicerSuperBuildLoadableExtensionTemplateModule.h
  qSlicerSuperBuildLoadableExtensionTemplateModuleWidget.cxx
  qSlicerSuperBuildLoadableExtensionTemplateModuleWidget.h
  )

# Headers that should run through moc
set(qt_module_MOC_SRCS
  qSlicerSuperBuildLoadableExtensionTemplateModule.h
  qSlicerSuperBuildLoadableExtensionTemplateModuleWidget.h
  )

# UI files
set(qt_module_UI_SRCS
  Resources/UI/qSlicerSuperBuildLoadableExtensionTemplateModule.ui
)

# Additional Target libraries
set(qt_module_target_libraries
  vtkSlicerSuperBuildLoadableExtensionTemplateModuleLogic
  )

# Resources
set(qt_module_resources
  Resources/qSlicerSuperBuildLoadableExtensionTemplateModule.qrc
)

#-----------------------------------------------------------------------------
# Build
#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${EXTENSION_NAME}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
# Generate extension description file '<EXTENSION_NAME>.s4ext'
#-----------------------------------------------------------------------------
include(SlicerFunctionGenerateExtensionDescription)
slicerFunctionGenerateExtensionDescription(
  EXTENSION_NAME ${EXTENSION_NAME}
  EXTENSION_BUILD_SUBDIRECTORY ${EXTENSION_BUILD_SUBDIRECTORY}
  EXTENSION_CATEGORY ${EXTENSION_CATEGORY}
  EXTENSION_STATUS ${EXTENSION_STATUS}
  EXTENSION_HOMEPAGE ${EXTENSION_HOMEPAGE}
  EXTENSION_DESCRIPTION ${EXTENSION_DESCRIPTION}
  EXTENSION_DEPENDS ${EXTENSION_DEPENDS}
  DESTINATION_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SLICER_WC_REVISION ${Slicer_WC_REVISION}
  SLICER_WC_ROOT ${Slicer_WC_ROOT}
  )

#-----------------------------------------------------------------------------
# Packaging
#-----------------------------------------------------------------------------
# Current project
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
# External projects
#set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${<EP>_DIR};<EP>;RuntimeLibraries;/")
INCLUDE(${Slicer_EXTENSION_CPACK})

