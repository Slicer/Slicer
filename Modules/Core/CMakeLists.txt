project(qSlicerModulesCore)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#
set(KIT_export_directive "Q_SLICER_MODULES_CORE_EXPORT")

# Additional directories to include
set(KIT_include_directories
  ${qSlicerBaseQTGUI_SOURCE_DIR}
  ${qSlicerBaseQTGUI_BINARY_DIR}
  ${MRMLLogic_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/EventBroker
  )

# Source files
set(KIT_SRCS
  qSlicerCoreModule.cxx
  qSlicerCoreModule.h

  qSlicerCoreModuleFactory.cxx
  qSlicerCoreModuleFactory.h

  EventBroker/qSlicerEventBrokerModule.cxx
  EventBroker/qSlicerEventBrokerModule.h
  EventBroker/qSlicerEventBrokerModuleWidget.cxx
  EventBroker/qSlicerEventBrokerModuleWidget.h
  )

# Headers that should run through moc
set(KIT_MOC_SRCS
  qSlicerCoreModule.h

  EventBroker/qSlicerEventBrokerModule.h
  EventBroker/qSlicerEventBrokerModuleWidget.h
  )

# UI files
set(KIT_UI_SRCS
  Resources/UI/qSlicerEventBrokerModuleWidget.ui
  )

# Additional Target libraries (Qt libs already included)
set(KIT_target_libraries
  # ${QT_LIBRARIES} # Not needed: All dependencies are transitively satisfied by other targets
  qSlicerBaseQTGUI
  )

# Resources
set(KIT_resources
  )

SlicerMacroBuildBaseQtLibrary(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${KIT_export_directive}
  INCLUDE_DIRECTORIES ${KIT_include_directories}
  SRCS ${KIT_SRCS}
  MOC_SRCS ${KIT_MOC_SRCS}
  UI_SRCS ${KIT_UI_SRCS}
  TARGET_LIBRARIES ${KIT_target_libraries}
  RESOURCES ${KIT_resources}
  )

# Plugin
#add_subdirectory(Plugins)

# Testing
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
