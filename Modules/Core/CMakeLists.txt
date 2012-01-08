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
  ${CMAKE_CURRENT_SOURCE_DIR}/Cameras
  ${CMAKE_CURRENT_SOURCE_DIR}/EventBroker
  ${CMAKE_CURRENT_SOURCE_DIR}/ROI
  )

# Source files
set(KIT_SRCS
  qSlicerCoreModule.cxx
  qSlicerCoreModule.h

  qSlicerCoreModuleFactory.cxx
  qSlicerCoreModuleFactory.h

  Cameras/qSlicerCamerasModule.cxx
  Cameras/qSlicerCamerasModule.h
  Cameras/qSlicerCamerasModuleWidget.cxx
  Cameras/qSlicerCamerasModuleWidget.h

  EventBroker/qSlicerEventBrokerModule.cxx
  EventBroker/qSlicerEventBrokerModule.h
  EventBroker/qSlicerEventBrokerModuleWidget.cxx
  EventBroker/qSlicerEventBrokerModuleWidget.h

  qSlicerFiducialsIO.cxx
  qSlicerFiducialsIO.h

  ROI/qSlicerROIModule.cxx
  ROI/qSlicerROIModule.h
  ROI/qSlicerROIModuleWidget.cxx
  ROI/qSlicerROIModuleWidget.h
  )

# Headers that should run through moc
set(KIT_MOC_SRCS
  qSlicerCoreModule.h

  Cameras/qSlicerCamerasModule.h
  Cameras/qSlicerCamerasModuleWidget.h

  EventBroker/qSlicerEventBrokerModule.h
  EventBroker/qSlicerEventBrokerModuleWidget.h

  qSlicerFiducialsIO.h

  ROI/qSlicerROIModule.h
  ROI/qSlicerROIModuleWidget.h

  )

# UI files
set(KIT_UI_SRCS
  Resources/UI/qSlicerCamerasModule.ui
  Resources/UI/qSlicerEventBrokerModule.ui
  Resources/UI/qSlicerROIModule.ui
)

# Additional Target libraries (QT libs already included)
set(KIT_target_libraries
  qSlicerBaseQTGUI
  )

# Resources
set(KIT_resources
  Resources/qSlicerModulesCore.qrc
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
