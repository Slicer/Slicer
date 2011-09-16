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
  qSlicerQListQVariantMapMetaType.h
  qSlicerSaveDataDialog.cxx
  qSlicerSaveDataDialog.h
  qSlicerSettingsModulesPanel.cxx
  qSlicerSettingsModulesPanel.h
  qSlicerSettingsGeneralPanel.cxx
  qSlicerSettingsGeneralPanel.h
  qSlicerStyle.cxx
  qSlicerStyle.h
  qSlicerWidget.cxx
  qSlicerWidget.h
  )

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
  qSlicerSettingsGeneralPanel.h
  qSlicerWidget.h
  )

if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_MOC_SRCS
    qSlicerScriptedLoadableModule.h
    qSlicerScriptedLoadableModuleWidget.h
    )
endif()

# PythonQt decorator
if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_MOC_SRCS
    qSlicerBaseQTGUIPythonQtDecorators.h
    qSlicerSettingsPythonPanel.h
    )
endif()

# UI files
set(KIT_UI_SRCS
  Resources/UI/qSlicerActionsDialog.ui
  Resources/UI/qSlicerDataDialog.ui
  Resources/UI/qSlicerExtensionsWizardWelcomePage.ui
  Resources/UI/qSlicerExtensionsWizardOverviewPage.ui
  Resources/UI/qSlicerExtensionsWizardRestartPage.ui
  Resources/UI/qSlicerModelsDialog.ui
  Resources/UI/qSlicerModulePanel.ui
  Resources/UI/qSlicerSaveDataDialog.ui
  Resources/UI/qSlicerSettingsModulesPanel.ui
  Resources/UI/qSlicerSettingsGeneralPanel.ui
)

# PythonQt decorator
if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_UI_SRCS
  Resources/UI/qSlicerSettingsPythonPanel.ui
  )
endif()

# Additional Target libraries (QT libs already included)
set(KIT_target_libraries
  MRMLDisplayableManager
  qCDashAPI
  qSlicerBaseQTCore
  qMRMLWidgets
  )

# Python wrap
if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_target_libraries
    CTKScriptingPythonWidgets
    CTKWidgetsPythonQt
    CTKDICOMWidgetsPythonQt
    CTKScriptingPythonWidgetsPythonQt
    CTKVisualizationVTKWidgetsPythonQt
    )
  if(qMRMLWidgets_WRAP_PYTHON)
    list(APPEND KIT_target_libraries
      qMRMLWidgetsPythonQt
    )
  endif()
endif()

# Resources
set(KIT_resources
  Resources/qSlicerBaseQTGUI.qrc
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

# HACK - Export library qSlicerBaseQTCorePythonQt so that
#        command line module (ie EMSegmentCommandLine) can
#        initialize both python environment and python manager.
set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS qSlicerBaseQTGUIPythonQt)

#-----------------------------------------------------------------------------
# Slicer icon engine
#-----------------------------------------------------------------------------
set(MY_LIBRARY_EXPORT_DIRECTIVE "Q_SLICER_BASE_QTGUI_ICON_ENGINE_EXPORT")
set(MY_EXPORT_HEADER_PREFIX qSlicerIconEnginePlugin)
set(MY_LIBNAME qSlicerIconEnginePlugin)

configure_file(
  ${Slicer_SOURCE_DIR}/qSlicerExport.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
  )
QT4_WRAP_CPP(MOC_qSlicerIconEnginePlugin qSlicerIconEnginePlugin.h)
add_library(qSlicerIconEnginePlugin
  qSlicerIconEnginePlugin.cxx
  qSlicerIconEnginePlugin.h
  ${MOC_qSlicerIconEnginePlugin})

target_link_libraries(qSlicerIconEnginePlugin
  ${QT_LIBRARIES}
  ${CTK_LIBRARIES})
  
# Set iconengine output path
set_target_properties(qSlicerIconEnginePlugin PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_LIB_DIR}/iconengines"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_LIB_DIR}/iconengines"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_LIB_DIR}/iconengines"
  )

# Install rules
install(TARGETS qSlicerIconEnginePlugin
  RUNTIME DESTINATION ${Slicer_INSTALL_LIB_DIR}/iconengines COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR}/iconengines COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR}/iconengines COMPONENT Development
  )
# Since Qt expects the plugin to be directly located under the
# directory '<Slicer_LIB_DIR>/iconengines', let's copy them.
if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
  get_target_property(FILE_PATH qSlicerIconEnginePlugin LOCATION)

  add_custom_command(
    TARGET qSlicerIconEnginePlugin
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${FILE_PATH}" ${CMAKE_BINARY_DIR}/${Slicer_LIB_DIR}/iconengines/${CMAKE_SHARED_LIBRARY_PREFIX}qSlicerIconEnginePlugin${CMAKE_BUILD_TYPE}${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
endif()

# Qt Designer Plugins
add_subdirectory(DesignerPlugins)

# Testing
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
