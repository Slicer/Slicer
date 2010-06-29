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
  qSlicerAbstractModulePanel.cxx
  qSlicerAbstractModulePanel.h
  qSlicerAbstractModuleWidget.cxx
  qSlicerAbstractModuleWidget.h
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
  qSlicerModulePanel.cxx
  qSlicerModulePanel.h
  qSlicerModuleSelectorWidget.cxx
  qSlicerModuleSelectorWidget.h
  qSlicerSaveDataDialog.cxx
  qSlicerSaveDataDialog.h
  qSlicerSliceViewWidget.cxx
  qSlicerSliceViewWidget.h
  qSlicerSliceViewWidget_p.h
  qSlicerWidget.cxx
  qSlicerWidget.h
  )

# Headers that should run through moc
SET(KIT_MOC_SRCS
  qSlicerAbstractModulePanel.h
  qSlicerAbstractModuleWidget.h
  qSlicerApplication.h
  qSlicerDataDialog.h
  qSlicerDataDialog_p.h
  qSlicerFileDialog.h
  qSlicerIOManager.h
  qSlicerIOOptionsWidget.h
  qSlicerLayoutManager.h
  qSlicerLayoutManager_p.h
  qSlicerModulePanel.h
  qSlicerModuleSelectorWidget.h
  qSlicerModuleSelectorWidget_p.h
  qSlicerSaveDataDialog.h
  qSlicerSaveDataDialog_p.h
  qSlicerSliceViewWidget.h
  qSlicerSliceViewWidget_p.h
  qSlicerWidget.h
  )
  
# PythonQt decorator
IF(Slicer3_USE_PYTHONQT)
  LIST(APPEND KIT_MOC_SRCS qSlicerBaseQTGUIPythonQtDecorators.h)
ENDIF(Slicer3_USE_PYTHONQT)

# UI files
SET(KIT_UI_SRCS
  Resources/UI/qSlicerDataDialog.ui
  Resources/UI/qSlicerModulePanel.ui
  Resources/UI/qSlicerModuleSelectorWidget.ui
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
