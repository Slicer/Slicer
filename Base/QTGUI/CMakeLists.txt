PROJECT(qSlicerBaseQTGUI)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(KIT_export_directive "Q_SLICER_BASE_QTGUI_EXPORT")

# Additional directories to include
SET(KIT_include_directories
  ${qSlicerBaseQTBase_SOURCE_DIR}
  ${qSlicerBaseQTBase_BINARY_DIR}
  ${qSlicerBaseQTCore_SOURCE_DIR}
  ${qSlicerBaseQTCore_BINARY_DIR}
  )
  
# Source files
SET(KIT_SRCS
  qSlicerAbstractModulePanel.cxx
  qSlicerAbstractModulePanel.h
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
  qSlicerModulePanel.cxx
  qSlicerModulePanel.h
  qSlicerModuleSelectorWidget.cxx
  qSlicerModuleSelectorWidget.h
  qSlicerSaveDataDialog.cxx
  qSlicerSaveDataDialog.h
  )

# Headers that should run through moc
SET(KIT_MOC_SRCS
  qSlicerAbstractModulePanel.h
  qSlicerApplication.h
  qSlicerDataDialog.h
  qSlicerDataDialog_p.h
  qSlicerFileDialog.h
  qSlicerIOManager.h
  qSlicerIOOptionsWidget.h
  qSlicerModulePanel.h
  qSlicerModuleSelectorWidget.h
  qSlicerModuleSelectorWidget_p.h
  qSlicerSaveDataDialog.h
  qSlicerSaveDataDialog_p.h
  )

# UI files
SET(KIT_UI_SRCS
  Resources/UI/qSlicerDataDialog.ui
  Resources/UI/qSlicerModulePanel.ui
  Resources/UI/qSlicerModuleSelectorWidget.ui
  Resources/UI/qSlicerSaveDataDialog.ui
)

# Additional Target libraries (QT libs already included)
SET(KIT_target_libraries
  qSlicerBaseQTBase
  qSlicerBaseQTCore
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
