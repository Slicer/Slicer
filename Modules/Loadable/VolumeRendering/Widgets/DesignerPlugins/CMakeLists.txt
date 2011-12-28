project(qSlicer${qt_module_name}ModuleWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

# The header '${module_widgets_name}Export.h' will be automatically configured.
set(module_widgets_export_directive "Q_VOLUMERENDERING_MODULE_WIDGETS_PLUGIN_EXPORT")

# Source files
set(designer_plugin_SRCS
  qMRMLVolumePropertyNodeWidgetPlugin.cxx
  qMRMLVolumePropertyNodeWidgetPlugin.h
  qSlicerVolumeRenderingModuleWidgetsAbstractPlugin.cxx
  qSlicerVolumeRenderingModuleWidgetsAbstractPlugin.h
  qSlicerVolumeRenderingModuleWidgetsPlugin.cxx
  qSlicerVolumeRenderingModuleWidgetsPlugin.h

  qSlicerPresetComboBoxPlugin.cxx
  qSlicerPresetComboBoxPlugin.h
  )


# Headers that should run through moc
set(designer_plugin_MOC_SRCS
  qSlicerVolumeRenderingModuleWidgetsPlugin.h

  qMRMLVolumePropertyNodeWidgetPlugin.h
  qSlicerPresetComboBoxPlugin.h
  )

set(designer_plugin_TARGET_LIBRARIES
  qSlicerVolumeRenderingModuleWidgets
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${module_widgets_export_directive}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )

