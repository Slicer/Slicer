PROJECT(qSlicerBaseQTGUI)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(KIT_export_directive "Q_SLICER_BASE_QTGUI_EXPORT")

# Additional directories to include
SET(KIT_include_directories
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  ${MRMLDisplayableManager_SOURCE_DIR}
  ${MRMLDisplayableManager_BINARY_DIR}
  ${qSlicerBaseQTCore_SOURCE_DIR}
  ${qSlicerBaseQTCore_BINARY_DIR}
  )

# Source files
SET(KIT_SRCS
  qSlicerAbstractModule.cxx
  qSlicerAbstractModule.h
  qSlicerAbstractModulePanel.cxx
  qSlicerAbstractModulePanel.h
  qSlicerAbstractModuleWidget.cxx
  qSlicerAbstractModuleWidget.h
  qSlicerActionsDialog.cxx
  qSlicerActionsDialog.h
  qSlicerApplication.cxx
  qSlicerApplication.h
  qSlicerCommandOptions.cxx
  qSlicerCommandOptions.h
  qSlicerDataDialog.cxx
  qSlicerDataDialog.h
  qSlicerFileDialog.cxx
  qSlicerFileDialog.h
  qSlicerIOManager.cxx
  qSlicerIOManager.h
  qSlicerIOOptionsWidget.cxx
  qSlicerIOOptionsWidget.h
  qSlicerLayoutManager.cxx
  qSlicerLayoutManager.h
  qSlicerLayoutManager_p.h
  qSlicerLoadableModule.cxx
  qSlicerLoadableModule.h

  qSlicerModelsDialog.cxx
  qSlicerModelsDialog.h
  qSlicerModelsDialog_p.h

  qSlicerModulePanel.cxx
  qSlicerModulePanel.h

  qSlicerModuleSelectorToolBar.cxx
  qSlicerModuleSelectorToolBar.h
  qSlicerMouseModeToolBar.cxx
  qSlicerMouseModeToolBar.h
  qSlicerMouseModeToolBar_p.h
  qSlicerSaveDataDialog.cxx
  qSlicerSaveDataDialog.h
  qSlicerStyle.cxx
  qSlicerStyle.h
  qSlicerWidget.cxx
  qSlicerWidget.h
  )

# Headers that should run through moc
SET(KIT_MOC_SRCS
  qSlicerAbstractModule.h
  qSlicerAbstractModulePanel.h
  qSlicerAbstractModuleWidget.h
  qSlicerActionsDialog.h
  qSlicerApplication.h
  qSlicerDataDialog.h
  qSlicerDataDialog_p.h
  qSlicerFileDialog.h
  qSlicerIOManager.h
  qSlicerIOOptionsWidget.h
  qSlicerLayoutManager.h
  qSlicerLayoutManager_p.h
  qSlicerLoadableModule.h

  qSlicerModelsDialog.h
  qSlicerModelsDialog_p.h

  qSlicerModulePanel.h
  qSlicerModuleSelectorToolBar.h
  qSlicerMouseModeToolBar.h
  qSlicerMouseModeToolBar_p.h
  qSlicerSaveDataDialog.h
  qSlicerSaveDataDialog_p.h
  qSlicerWidget.h
  )
  
# PythonQt decorator
IF(Slicer3_USE_PYTHONQT)
  LIST(APPEND KIT_MOC_SRCS qSlicerBaseQTGUIPythonQtDecorators.h)
ENDIF(Slicer3_USE_PYTHONQT)

# UI files
SET(KIT_UI_SRCS
  Resources/UI/qSlicerActionsDialog.ui 
  Resources/UI/qSlicerDataDialog.ui
  Resources/UI/qSlicerModelsDialog.ui
  Resources/UI/qSlicerModulePanel.ui
  Resources/UI/qSlicerSaveDataDialog.ui
)

# Additional Target libraries (QT libs already included)
SET(KIT_target_libraries
  MRMLDisplayableManager
  qSlicerBaseQTCore
  qMRMLWidgets
  )

# Python wrap
IF(Slicer3_USE_PYTHONQT)
  LIST(APPEND KIT_target_libraries
    CTKScriptingPythonWidgets
    )
ENDIF()

# Resources
SET(KIT_resources
  Resources/qSlicerBaseQTGUI.qrc
)

# Python wrap
IF(Slicer3_USE_PYTHONQT)

  # Sources
  LIST(APPEND KIT_SRCS
    qSlicerPythonManager.cxx
    qSlicerPythonManager.h
    )

  # Headers that should run through moc
  LIST(APPEND KIT_MOC_SRCS
    qSlicerPythonManager.h
    )

ENDIF(Slicer3_USE_PYTHONQT)

Slicer3_build_slicer_qtbase_library(
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
ADD_SUBDIRECTORY(Plugins)

# Testing
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
