#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Slicer Package
#------------------------------------------------------------------------------

IF (Slicer_SOURCE_DIR)
  MESSAGE("Slicer_INCLUDE_DIR: ${Slicer_INCLUDE_DIR}")
ELSE (Slicer_SOURCE_DIR)
  FIND_PACKAGE(Slicer REQUIRED)
  INCLUDE(${Slicer_USE_FILE})
ENDIF (Slicer_SOURCE_DIR)

#------------------------------------------------------------------------------
# Qt Package
#------------------------------------------------------------------------------
SET(minimum_required_qt_version "4.6.2")

# Module name (usually identique to the directory name)
FIND_PACKAGE(Qt4)
IF(NOT QT4_FOUND)
  MESSAGE(FATAL_ERROR "error: Qt >= ${minimum_required_qt_version} was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable.")
ENDIF()
SET(qt_module_name ExtensionTemplate)

# Check version, note that ${QT_VERSION_PATCH} could also be used
IF("${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}" VERSION_LESS "${minimum_required_qt_version}")
  MESSAGE(FATAL_ERROR "error: Slicer requires Qt >= ${minimum_required_qt_version} -- you cannot use Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}. ${extra_error_message}")
ENDIF()

MESSAGE(STATUS "Configuring Slicer with Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")

# Includes Qt headers
INCLUDE(${QT_USE_FILE})

# Add subdirectories
ADD_SUBDIRECTORY(Logic)

SET(qt_module_export_directive "Q_SLICER_QTMODULES_EXTENSIONTEMPLATE_EXPORT")
# Module name (usually identique to the directory name)
SET(qt_module_name "ExtensionTest")


# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

# Source files
SET(qt_module_SRCS
  qSlicerExtensionTemplateModule.cxx
  qSlicerExtensionTemplateModule.h
  qSlicerExtensionTemplateModuleWidget.cxx
  qSlicerExtensionTemplateModuleWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerExtensionTemplateModule.h
  qSlicerExtensionTemplateModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerExtensionTemplateModule.ui
)

# Additional Target libraries
SET(qt_module_target_libraries
  vtkSlicerExtensionTemplateModuleLogic
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerExtensionTemplateModule.qrc
)

slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )


if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif(BUILD_TESTING)
