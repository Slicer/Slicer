PROJECT(qSlicerBaseQTCoreModules)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#

SET(KIT_export_directive "Q_SLICER_BASE_QTCOREMODULES_EXPORT")

# Additional directories to include
SET(KIT_include_directories
  ${qSlicerBaseQTGUI_SOURCE_DIR}
  ${qSlicerBaseQTGUI_BINARY_DIR}
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  )

# Source files
SET(KIT_SRCS
  qSlicerCoreModule.cxx
  qSlicerCoreModule.h

  qSlicerCoreModuleFactory.cxx
  qSlicerCoreModuleFactory.h

  qSlicerCamerasModule.cxx
  qSlicerCamerasModule.h
  qSlicerCamerasModuleWidget.cxx
  qSlicerCamerasModuleWidget.h

  qSlicerColorIO.cxx
  qSlicerColorIO.h
  qSlicerColorsModule.cxx
  qSlicerColorsModule.h
  qSlicerColorsModuleWidget.cxx
  qSlicerColorsModuleWidget.h

  qSlicerDataModule.cxx
  qSlicerDataModule.h
  qSlicerDataModuleWidget.cxx
  qSlicerDataModuleWidget.h

  qSlicerFiducialsIO.cxx
  qSlicerFiducialsIO.h

  qSlicerMRMLTreeModule.cxx
  qSlicerMRMLTreeModule.h
  qSlicerMRMLTreeModuleWidget.cxx
  qSlicerMRMLTreeModuleWidget.h

  qSlicerROIModule.cxx
  qSlicerROIModule.h
  qSlicerROIModuleWidget.cxx
  qSlicerROIModuleWidget.h

  qSlicerScalarOverlayIO.cxx
  qSlicerScalarOverlayIO.h
  qSlicerScalarOverlayIOOptionsWidget.cxx
  qSlicerScalarOverlayIOOptionsWidget.h

  qSlicerSliceControllerModule.cxx
  qSlicerSliceControllerModule.h
  qSlicerSliceControllerModuleWidget.cxx
  qSlicerSliceControllerModuleWidget.h

  qSlicerTransformsIO.cxx
  qSlicerTransformsIO.h
  qSlicerTransformsModule.cxx
  qSlicerTransformsModule.h
  qSlicerTransformsModuleWidget.cxx
  qSlicerTransformsModuleWidget.h
  )

# Headers that should run through moc
SET(KIT_MOC_SRCS
  qSlicerCoreModule.h

  qSlicerCamerasModule.h
  qSlicerCamerasModuleWidget.h

  qSlicerColorIO.h
  qSlicerColorsModule.h
  qSlicerColorsModuleWidget.h

  qSlicerDataModule.h
  qSlicerDataModuleWidget.h

  qSlicerFiducialsIO.h

  qSlicerMRMLTreeModule.h
  qSlicerMRMLTreeModuleWidget.h

  qSlicerROIModule.h
  qSlicerROIModuleWidget.h

  qSlicerScalarOverlayIO.h
  qSlicerScalarOverlayIOOptionsWidget.h

  qSlicerSliceControllerModule.h
  qSlicerSliceControllerModuleWidget.h

  qSlicerTransformsIO.h
  qSlicerTransformsModule.h
  qSlicerTransformsModuleWidget.h
  )

# UI files
SET(KIT_UI_SRCS
  Resources/UI/qSlicerCamerasModule.ui
  Resources/UI/qSlicerColorsModule.ui
  Resources/UI/qSlicerDataModule.ui
  Resources/UI/qSlicerMRMLTreeModule.ui
  Resources/UI/qSlicerROIModule.ui
  Resources/UI/qSlicerScalarOverlayIOOptionsWidget.ui
  Resources/UI/qSlicerSliceControllerModule.ui
  Resources/UI/qSlicerTransformsModule.ui
)

# Additional Target libraries (QT libs already included)
SET(KIT_target_libraries
  qSlicerBaseQTGUI
  )

# Resources
SET(KIT_resources
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
#ADD_SUBDIRECTORY(Plugins)

# Testing
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
