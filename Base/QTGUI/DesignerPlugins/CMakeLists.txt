set(KIT ${PROJECT_NAME}DesignerPlugins)
project(${KIT})

#
# See CTK/CMake/ctkMacroBuildQtPlugin.cmake for details
#

# The header '${KIT}Export.h' will be automatically configured.
set(${KIT}_EXPORT "Q_SLICER_DESIGNER_PLUGINS_EXPORT")

# Source files
set(${KIT}_SRCS
  qSlicerQTGUIAbstractPlugin.cxx
  qSlicerQTGUIAbstractPlugin.h
  qSlicerQTGUIPlugins.cxx
  qSlicerQTGUIPlugins.h

  qSlicerDirectoryListViewPlugin.cxx
  qSlicerDirectoryListViewPlugin.h
  qSlicerModulePanelPlugin.cxx
  qSlicerModulePanelPlugin.h
  qSlicerModulesListViewPlugin.cxx
  qSlicerModulesListViewPlugin.h
  qSlicerMouseModeToolBarPlugin.cxx
  qSlicerMouseModeToolBarPlugin.h
  qSlicerWidgetPlugin.cxx
  qSlicerWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerBaseQTGUI
  qSlicerBaseQTCLI
  )

ctkMacroBuildQtDesignerPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "Core-Base"
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

target_precompile_headers(${PROJECT_NAME} REUSE_FROM PrecompileQtCoreHeaders)
target_precompile_headers(${PROJECT_NAME} REUSE_FROM PrecompileQtWidgetsHeaders)
