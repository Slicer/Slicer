project(qSlicerBaseQTGUIPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

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
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )
