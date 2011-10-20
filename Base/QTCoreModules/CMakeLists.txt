project(qSlicerBaseQTCoreModules)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#

set(KIT_export_directive "Q_SLICER_BASE_QTCOREMODULES_EXPORT")

# Additional directories to include
set(KIT_include_directories
  ${qSlicerBaseQTGUI_SOURCE_DIR}
  ${qSlicerBaseQTGUI_BINARY_DIR}
  ${MRMLLogic_INCLUDE_DIRS}
  )

# Source files
set(KIT_SRCS
  qSlicerCoreModule.cxx
  qSlicerCoreModule.h

  qSlicerCoreModuleFactory.cxx
  qSlicerCoreModuleFactory.h

  qSlicerCamerasModule.cxx
  qSlicerCamerasModule.h
  qSlicerCamerasModuleWidget.cxx
  qSlicerCamerasModuleWidget.h

  qSlicerEventBrokerModule.cxx
  qSlicerEventBrokerModule.h
  qSlicerEventBrokerModuleWidget.cxx
  qSlicerEventBrokerModuleWidget.h

  qSlicerFiducialsIO.cxx
  qSlicerFiducialsIO.h

  qSlicerROIModule.cxx
  qSlicerROIModule.h
  qSlicerROIModuleWidget.cxx
  qSlicerROIModuleWidget.h

  qSlicerScalarOverlayIO.cxx
  qSlicerScalarOverlayIO.h
  qSlicerScalarOverlayIOOptionsWidget.cxx
  qSlicerScalarOverlayIOOptionsWidget.h

  qSlicerViewControllersModule.cxx
  qSlicerViewControllersModule.h
  qSlicerViewControllersModuleWidget.cxx
  qSlicerViewControllersModuleWidget.h

  qSlicerTransformsIO.cxx
  qSlicerTransformsIO.h
  qSlicerTransformsModule.cxx
  qSlicerTransformsModule.h
  qSlicerTransformsModuleWidget.cxx
  qSlicerTransformsModuleWidget.h
  )

# Headers that should run through moc
set(KIT_MOC_SRCS
  qSlicerCoreModule.h

  qSlicerCamerasModule.h
  qSlicerCamerasModuleWidget.h

  qSlicerEventBrokerModule.h
  qSlicerEventBrokerModuleWidget.h

  qSlicerFiducialsIO.h

  qSlicerROIModule.h
  qSlicerROIModuleWidget.h

  qSlicerScalarOverlayIO.h
  qSlicerScalarOverlayIOOptionsWidget.h

  qSlicerViewControllersModule.h
  qSlicerViewControllersModuleWidget.h

  qSlicerTransformsIO.h
  qSlicerTransformsModule.h
  qSlicerTransformsModuleWidget.h
  )

# UI files
set(KIT_UI_SRCS
  Resources/UI/qSlicerCamerasModule.ui
  Resources/UI/qSlicerEventBrokerModule.ui
  Resources/UI/qSlicerROIModule.ui
  Resources/UI/qSlicerScalarOverlayIOOptionsWidget.ui
  Resources/UI/qSlicerViewControllersModule.ui
  Resources/UI/qSlicerTransformsModule.ui
)

# Additional Target libraries (QT libs already included)
set(KIT_target_libraries
  qSlicerBaseQTGUI
  )

# Resources
set(KIT_resources
  Resources/qSlicerBaseQTCoreModules.qrc
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
