PROJECT(qSlicerBaseQTGUI)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
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
  
  qSlicerExtensionsWizard.cxx
  qSlicerExtensionsWizard.h
  qSlicerExtensionsWizardOverviewPage.cxx
  qSlicerExtensionsWizardOverviewPage.h
  qSlicerExtensionsWizardRestartPage.cxx
  qSlicerExtensionsWizardRestartPage.h
  qSlicerExtensionsWizardWelcomePage.cxx
  qSlicerExtensionsWizardWelcomePage.h

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

  qSlicerModulesMenu.cxx
  qSlicerModulesMenu.h

  qSlicerModulePanel.cxx
  qSlicerModulePanel.h

  qSlicerModuleSelectorToolBar.cxx
  qSlicerModuleSelectorToolBar.h
  qSlicerMouseModeToolBar.cxx
  qSlicerMouseModeToolBar.h
  qSlicerMouseModeToolBar_p.h
  qSlicerSaveDataDialog.cxx
  qSlicerSaveDataDialog.h
  qSlicerSettingsModulesPanel.cxx
  qSlicerSettingsModulesPanel.h
  qSlicerSettingsPanel.cxx
  qSlicerSettingsPanel.h
  qSlicerStyle.cxx
  qSlicerStyle.h
  qSlicerWidget.cxx
  qSlicerWidget.h
  )

IF(Slicer_USE_PYTHONQT)
  LIST(APPEND KIT_SRCS
    qSlicerScriptedLoadableModuleFactory.cxx
    qSlicerScriptedLoadableModuleFactory.h
    qSlicerScriptedLoadableModuleWidget.cxx
    qSlicerScriptedLoadableModuleWidget.h
    qSlicerScriptedLoadableModule.cxx
    qSlicerScriptedLoadableModule.h
    )
ENDIF()

# Since the wrapper doesn't consider hierarchy of classes, let's exclude
# classes deriving from pure abstract class and remaining abstract pure
SET_SOURCE_FILES_PROPERTIES(
  qSlicerAbstractModule.h
  qSlicerAbstractModulePanel.h
  qSlicerAbstractModuleWidget.h
  qSlicerFileDialog.h
  qSlicerLoadableModule.h
  WRAP_EXCLUDE
  )

# Headers that should run through moc
SET(KIT_MOC_SRCS
  qSlicerAbstractModule.h
  qSlicerAbstractModulePanel.h
  qSlicerAbstractModuleWidget.h
  qSlicerActionsDialog.h
  qSlicerApplication.h
  qSlicerCommandOptions.h
  qSlicerDataDialog.h
  qSlicerDataDialog_p.h
  qSlicerExtensionsWizard.h
  qSlicerExtensionsWizardOverviewPage.h
  qSlicerExtensionsWizardRestartPage.h
  qSlicerExtensionsWizardWelcomePage.h
  qSlicerFileDialog.h
  qSlicerIOManager.h
  qSlicerIOOptionsWidget.h
  qSlicerLayoutManager.h
  qSlicerLoadableModule.h

  qSlicerModelsDialog.h
  qSlicerModelsDialog_p.h

  qSlicerModulesMenu.h
  qSlicerModulePanel.h
  qSlicerModuleSelectorToolBar.h
  qSlicerMouseModeToolBar.h
  qSlicerMouseModeToolBar_p.h
  qSlicerSaveDataDialog.h
  qSlicerSaveDataDialog_p.h
  qSlicerSettingsModulesPanel.h
  qSlicerSettingsPanel.h
  qSlicerWidget.h
  )
  
# PythonQt decorator
IF(Slicer_USE_PYTHONQT)
  LIST(APPEND KIT_MOC_SRCS
    qSlicerBaseQTGUIPythonQtDecorators.h
    qSlicerScriptedLoadableModule.h
    qSlicerScriptedLoadableModuleWidget.h
    )
ENDIF(Slicer_USE_PYTHONQT)

# UI files
SET(KIT_UI_SRCS
  Resources/UI/qSlicerActionsDialog.ui 
  Resources/UI/qSlicerDataDialog.ui
  Resources/UI/qSlicerExtensionsWizardWelcomePage.ui
  Resources/UI/qSlicerExtensionsWizardOverviewPage.ui
  Resources/UI/qSlicerExtensionsWizardRestartPage.ui
  Resources/UI/qSlicerModelsDialog.ui
  Resources/UI/qSlicerModulePanel.ui
  Resources/UI/qSlicerSaveDataDialog.ui
  Resources/UI/qSlicerSettingsModulesPanel.ui
  Resources/UI/qSlicerSettingsPanel.ui
)

# Additional Target libraries (QT libs already included)
SET(KIT_target_libraries
  MRMLDisplayableManager
  qSlicerBaseQTCore
  qMRMLWidgets
  )

# Python wrap
IF(Slicer_USE_PYTHONQT)

  LIST(APPEND KIT_target_libraries
    CTKScriptingPythonWidgets
    CTKWidgetsPythonQt
    CTKScriptingPythonWidgetsPythonQt
    CTKVisualizationVTKWidgetsPythonQt
    )
  IF(qMRMLWidgets_WRAP_PYTHON)
    LIST(APPEND KIT_target_libraries
      qMRMLWidgetsPythonQt
    )
  ENDIF()
ENDIF()

# Resources
SET(KIT_resources
  Resources/qSlicerBaseQTGUI.qrc
)

# Python wrap
IF(Slicer_USE_PYTHONQT)

  # Sources
  LIST(APPEND KIT_SRCS
    qSlicerPythonManager.cxx
    qSlicerPythonManager.h
    )

  # Headers that should run through moc
  LIST(APPEND KIT_MOC_SRCS
    qSlicerPythonManager.h
    )

ENDIF(Slicer_USE_PYTHONQT)

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

# Slicer icon engine
SET(MY_LIBRARY_EXPORT_DIRECTIVE "Q_SLICER_BASE_QTGUI_ICON_ENGINE_EXPORT")
SET(MY_EXPORT_HEADER_PREFIX qSlicerIconEnginePlugin)
SET(MY_LIBNAME qSlicerIconEnginePlugin)

CONFIGURE_FILE(
  ${Slicer_SOURCE_DIR}/qSlicerExport.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
  )
QT4_WRAP_CPP(MOC_qSlicerIconEnginePlugin qSlicerIconEnginePlugin.h)
ADD_LIBRARY(qSlicerIconEnginePlugin
  qSlicerIconEnginePlugin.cxx
  qSlicerIconEnginePlugin.h
  ${MOC_qSlicerIconEnginePlugin})

TARGET_LINK_LIBRARIES(qSlicerIconEnginePlugin
  ${QT_LIBRARIES}
  ${CTK_LIBRARIES})

# Install rules
INSTALL(TARGETS qSlicerIconEnginePlugin
  RUNTIME DESTINATION lib/Slicer3/iconengines COMPONENT RuntimeLibraries
  LIBRARY DESTINATION lib/Slicer3/iconengines COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/Slicer3/iconengines COMPONENT Development
  )
# Since Qt expects the plugin to be directly located under the
# directory 'lib/Slicer3/iconengines', let's copy them.

IF(NOT CMAKE_CFG_INTDIR STREQUAL ".")
  GET_TARGET_PROPERTY(FILE_PATH qSlicerIconEnginePlugin LOCATION)
  
  ADD_CUSTOM_COMMAND(
    TARGET qSlicerIconEnginePlugin
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${FILE_PATH}" ${CMAKE_BINARY_DIR}/lib/Slicer3/iconengines/${CMAKE_SHARED_LIBRARY_PREFIX}qSlicerIconEnginePlugin${CMAKE_BUILD_TYPE}${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
ENDIF()

# Plugin
ADD_SUBDIRECTORY(Plugins)

# Testing
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
