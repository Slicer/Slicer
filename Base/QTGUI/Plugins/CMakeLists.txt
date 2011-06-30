project(qSlicerBaseQTGUIPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# The header '${module_widgets_name}Export.h' will be automatically configured.
set(module_widgets_export_directive "Q_SLICER_BASE_QTGUI_PLUGIN_EXPORT")

# Source files
set(designer_plugin_SRCS
  qSlicerQTGUIAbstractPlugin.cxx
  qSlicerQTGUIAbstractPlugin.h
  qSlicerQTGUIPlugins.cxx
  qSlicerQTGUIPlugins.h

  qSlicerModulePanelPlugin.cxx
  qSlicerModulePanelPlugin.h
  qSlicerMouseModeToolBarPlugin.cxx
  qSlicerMouseModeToolBarPlugin.h
  qSlicerWidgetPlugin.cxx
  qSlicerWidgetPlugin.h
  )

# Headers that should run through moc
set(designer_plugin_MOC_SRCS
  qSlicerQTGUIPlugins.h

  qSlicerModulePanelPlugin.h
  qSlicerMouseModeToolBarPlugin.h
  qSlicerWidgetPlugin.h
  )

set(designer_plugin_TARGET_LIBRARIES
  qSlicerBaseQTCLI
  qSlicerBaseQTGUI
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${module_widgets_export_directive}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )
