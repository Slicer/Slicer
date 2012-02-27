project(qSlicerBaseQTGUI)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#

set(KIT_export_directive "Q_SLICER_BASE_QTGUI_EXPORT")

# Additional directories to include
set(KIT_include_directories
  ${MRMLLogic_INCLUDE_DIRS}
  ${MRMLDisplayableManager_INCLUDE_DIRS}
  ${qSlicerBaseQTCore_SOURCE_DIR}
  ${qSlicerBaseQTCore_BINARY_DIR}
  )

# Source files
set(KIT_SRCS
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
  qSlicerDirectoryListView.cxx
  qSlicerDirectoryListView.h

  qSlicerFileDialog.cxx
  qSlicerFileDialog.h
  qSlicerIOManager.cxx
  qSlicerIOManager.h
  qSlicerIOOptionsWidget.cxx
  qSlicerIOOptionsWidget.h
  qSlicerLayoutManager.cxx
  qSlicerLayoutManager.h
  qSlicerLoadableModule.cxx
  qSlicerLoadableModule.h

  qSlicerModelsDialog.cxx
  qSlicerModelsDialog.h
  qSlicerModelsDialog_p.h

  qSlicerModuleFactoryFilterModel.cxx
  qSlicerModuleFactoryFilterModel.h
  qSlicerModulesListView.cxx
  qSlicerModulesListView.h
  qSlicerModulesMenu.cxx
  qSlicerModulesMenu.h

  qSlicerModulePanel.cxx
  qSlicerModulePanel.h

  qSlicerModuleSelectorToolBar.cxx
  qSlicerModuleSelectorToolBar.h
  qSlicerMouseModeToolBar.cxx
  qSlicerMouseModeToolBar.h
  qSlicerMouseModeToolBar_p.h
  qSlicerQListQVariantMapMetaType.h
  qSlicerSaveDataDialog.cxx
  qSlicerSaveDataDialog.h
  qSlicerSettingsModulesPanel.cxx
  qSlicerSettingsModulesPanel.h
  qSlicerSettingsGeneralPanel.cxx
  qSlicerSettingsGeneralPanel.h
  qSlicerStyle.cxx
  qSlicerStyle.h
  qSlicerViewersToolBar.cxx
  qSlicerViewersToolBar.h
  qSlicerViewersToolBar_p.h
  qSlicerWidget.cxx
  qSlicerWidget.h
  )

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_SRCS
    qSlicerExtensionsInstallWidget.cxx
    qSlicerExtensionsInstallWidget.h
    qSlicerExtensionsManageWidget.cxx
    qSlicerExtensionsManageWidget.h
    qSlicerExtensionsManagerDialog.cxx
    qSlicerExtensionsManagerDialog.h
    qSlicerExtensionsManagerWidget.cxx
    qSlicerExtensionsManagerWidget.h
    qSlicerSettingsExtensionsPanel.cxx
    qSlicerSettingsExtensionsPanel.h
    )
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_SRCS
    qSlicerScriptedLoadableModuleFactory.cxx
    qSlicerScriptedLoadableModuleFactory.h
    qSlicerScriptedLoadableModuleWidget.cxx
    qSlicerScriptedLoadableModuleWidget.h
    qSlicerScriptedLoadableModule.cxx
    qSlicerScriptedLoadableModule.h
    )
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_SRCS
    qSlicerSettingsPythonPanel.cxx
    qSlicerSettingsPythonPanel.h
    )
endif()

# Since the wrapper doesn't consider hierarchy of classes, let's exclude
# classes deriving from pure abstract class and remaining abstract pure
set_source_files_properties(
  qSlicerAbstractModule.h
  qSlicerAbstractModulePanel.h
  qSlicerAbstractModuleWidget.h
  qSlicerFileDialog.h
  qSlicerLoadableModule.h
  WRAP_EXCLUDE
  )

# Headers that should run through moc
set(KIT_MOC_SRCS
  qSlicerAbstractModule.h
  qSlicerAbstractModulePanel.h
  qSlicerAbstractModuleWidget.h
  qSlicerActionsDialog.h
  qSlicerApplication.h
  qSlicerCommandOptions.h
  qSlicerDataDialog.h
  qSlicerDataDialog_p.h
  qSlicerDirectoryListView.h
  qSlicerFileDialog.h
  qSlicerIOManager.h
  qSlicerIOOptionsWidget.h
  qSlicerLayoutManager.h
  qSlicerLoadableModule.h

  qSlicerModelsDialog.h
  qSlicerModelsDialog_p.h

  qSlicerModuleFactoryFilterModel.h
  qSlicerModulesListView.h
  qSlicerModulesMenu.h
  qSlicerModulePanel.h
  qSlicerModuleSelectorToolBar.h
  qSlicerMouseModeToolBar.h
  qSlicerMouseModeToolBar_p.h
  qSlicerSaveDataDialog.h
  qSlicerSaveDataDialog_p.h
  qSlicerSettingsModulesPanel.h
  qSlicerSettingsGeneralPanel.h
  qSlicerViewersToolBar.h
  qSlicerViewersToolBar_p.h
  qSlicerWidget.h
  )

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_MOC_SRCS
    qSlicerExtensionsInstallWidget.h
    qSlicerExtensionsManageWidget.h
    qSlicerExtensionsManagerDialog.h
    qSlicerExtensionsManagerWidget.h
    qSlicerSettingsExtensionsPanel.h
    )
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_MOC_SRCS
    qSlicerScriptedLoadableModule.h
    qSlicerScriptedLoadableModuleWidget.h
    )
endif()

# PythonQt decorator
if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_MOC_SRCS
    qSlicerSettingsPythonPanel.h
    )
endif()

# UI files
set(KIT_UI_SRCS
  Resources/UI/qSlicerActionsDialog.ui
  Resources/UI/qSlicerDataDialog.ui
  Resources/UI/qSlicerModelsDialog.ui
  Resources/UI/qSlicerModulePanel.ui
  Resources/UI/qSlicerSaveDataDialog.ui
  Resources/UI/qSlicerSettingsModulesPanel.ui
  Resources/UI/qSlicerSettingsGeneralPanel.ui
)
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_UI_SRCS
    Resources/UI/qSlicerExtensionsButtonBox.ui
    Resources/UI/qSlicerExtensionsInstallWidget.ui
    Resources/UI/qSlicerExtensionsManageWidget.ui
    Resources/UI/qSlicerExtensionsManagerDialog.ui
    Resources/UI/qSlicerExtensionsManagerWidget.ui
    Resources/UI/qSlicerSettingsExtensionsPanel.ui
    )
endif()

# PythonQt decorator
if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_UI_SRCS
    Resources/UI/qSlicerSettingsPythonPanel.ui
  )
endif()

# Additional Target libraries (QT libs already included)
set(KIT_target_libraries
  qSlicerBaseQTCore
  qMRMLWidgets
  )

# Python wrap
if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_target_libraries
    CTKScriptingPythonWidgets
    )
endif()

# Resources
set(KIT_resources
  Resources/qSlicerBaseQTGUI.qrc
  ${qMRMLWidgets_SOURCE_DIR}/Resources/qMRMLWidgets.qrc
)

# Python wrap
if(Slicer_USE_PYTHONQT)

  # Sources
  list(APPEND KIT_SRCS
    qSlicerPythonManager.cxx
    qSlicerPythonManager.h
    )

  # Headers that should run through moc
  list(APPEND KIT_MOC_SRCS
    qSlicerPythonManager.h
    )

endif(Slicer_USE_PYTHONQT)

SlicerMacroBuildBaseQtLibrary(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${KIT_export_directive}
  INCLUDE_DIRECTORIES ${KIT_include_directories}
  SRCS ${KIT_SRCS}
  MOC_SRCS ${KIT_MOC_SRCS}
  UI_SRCS ${KIT_UI_SRCS}
  TARGET_LIBRARIES ${KIT_target_libraries}
  RESOURCES ${KIT_resources}
  WRAP_PYTHONQT
  )

#-----------------------------------------------------------------------------
# Slicer icon engine
#-----------------------------------------------------------------------------
ctkMacroBuildQtIconEnginesPlugin(
  NAME qSlicerIconEnginePlugin
  EXPORT_DIRECTIVE "Q_SLICER_BASE_QTGUI_ICON_ENGINE_EXPORT"
  SRCS qSlicerIconEnginePlugin.cxx qSlicerIconEnginePlugin.h
  MOC_SRCS qSlicerIconEnginePlugin.h
  TARGET_LIBRARIES ${QT_LIBRARIES} ${CTK_LIBRARIES}
  )

# Qt Designer Plugins
add_subdirectory(DesignerPlugins)

# Testing
# Note: Within both Testing/Cxx and Testing/Python, some code is useful to both Slicer and the
#       Extensions. For that reason, the Testing directory is added inconditionnally.
#       The code specific to Slicer will be excluded directly in either Testing/Python
#       or Testing/Cxx.
add_subdirectory(Testing)
