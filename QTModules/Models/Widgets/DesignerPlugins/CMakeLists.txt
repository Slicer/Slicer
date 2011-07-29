project(qSlicerModelsWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# The header '${module_widgets_name}Export.h' will be automatically configured.
set(module_widgets_export_directive "Q_SLICER_QTMODULES_MODELS_WIDGETS_PLUGIN_EXPORT")

# Source files
set(designer_plugin_SRCS
  qSlicerModelsWidgetsAbstractPlugin.cxx
  qSlicerModelsWidgetsAbstractPlugin.h
  qSlicerModelsWidgetsPlugin.cxx
  qSlicerModelsWidgetsPlugin.h

  qMRMLModelDisplayNodeWidgetPlugin.cxx
  qMRMLModelDisplayNodeWidgetPlugin.h
  )

# Headers that should run through moc
set(designer_plugin_MOC_SRCS
  qSlicerModelsWidgetsPlugin.h

  qMRMLModelDisplayNodeWidgetPlugin.h
  )

set(designer_plugin_TARGET_LIBRARIES
  qSlicerModelsModuleWidgets
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${module_widgets_export_directive}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )
