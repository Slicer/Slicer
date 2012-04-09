cmake_minimum_required(VERSION 2.8.7)

#-----------------------------------------------------------------------------
set(EXTENSION_NAME SuperBuildLoadableExtensionTemplate) # Do not use 'project()' - See SlicerConfig.cmake

#-----------------------------------------------------------------------------
set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Snapshot/Developers/Tutorials/ExtensionWriting")
set(EXTENSION_CATEGORY "Examples")
set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/SuperBuildLoadableExtensionTemplate/Resources/Icons/SuperBuildLoadableExtensionTemplate.png?revision=19437&view=co")
set(EXTENSION_STATUS "Beta")
set(EXTENSION_CONTRIBUTORS "Jean-Christophe Fillion-Robin (Kitware)")
set(EXTENSION_DESCRIPTION "This is an example of Qt loadable module using SuperBuild built as an extension")
set(EXTENSION_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/b/b3/Slicer-r19441-SuperBuildLoadableExtensionTemplate-screenshot.png")
set(EXTENSION_DEPENDS CLIExtensionTemplate) # Specified as a space separated list or 'NA' if any
set(EXTENSION_BUILD_SUBDIRECTORY inner-build)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})

  find_package(Git REQUIRED)

  include(SlicerEnableExtensionTesting)
endif()

#-----------------------------------------------------------------------------
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)
if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#-----------------------------------------------------------------------------
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(qt_module_export_directive "Q_SLICER_QTMODULES_SUPERBUILDLOADABLEEXTENSIONTEMPLATE_EXPORT")

set(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

set(qt_module_SRCS
  qSlicerSuperBuildLoadableExtensionTemplateModule.cxx
  qSlicerSuperBuildLoadableExtensionTemplateModule.h
  qSlicerSuperBuildLoadableExtensionTemplateModuleWidget.cxx
  qSlicerSuperBuildLoadableExtensionTemplateModuleWidget.h
  )

set(qt_module_MOC_SRCS
  qSlicerSuperBuildLoadableExtensionTemplateModule.h
  qSlicerSuperBuildLoadableExtensionTemplateModuleWidget.h
  )

set(qt_module_UI_SRCS
  Resources/UI/qSlicerSuperBuildLoadableExtensionTemplateModule.ui
  )

set(qt_module_target_libraries
  vtkSlicerSuperBuildLoadableExtensionTemplateModuleLogic
  )

set(qt_module_resources
  Resources/qSlicerSuperBuildLoadableExtensionTemplateModule.qrc
  )

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
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
#set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${<EP>_DIR};<EP>;RuntimeLibraries;/")
INCLUDE(${Slicer_EXTENSION_CPACK})
