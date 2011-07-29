project(qSlicer${qt_module_name}ModuleWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#
# The header '${module_widgets_name}Export.h' will be automatically configured.
set(module_widgets_export_directive "Q_SLICER_MODULE_ANNOTATIONS_WIDGETS_PLUGIN_EXPORT")

# Source files
set(designer_plugin_SRCS
  qSlicerAnnotationModuleWidgetsAbstractPlugin.cxx
  qSlicerAnnotationModuleWidgetsAbstractPlugin.h
  qSlicerAnnotationModuleWidgetsPlugin.cxx
  qSlicerAnnotationModuleWidgetsPlugin.h

  qMRMLAnnotationROIWidgetPlugin.cxx
  qMRMLAnnotationROIWidgetPlugin.h
  )


# Headers that should run through moc
set(designer_plugin_MOC_SRCS
  qSlicerAnnotationModuleWidgetsPlugin.h

  qMRMLAnnotationROIWidgetPlugin.h
  )

set(designer_plugin_TARGET_LIBRARIES
  qSlicerAnnotationsModuleWidgets
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${module_widgets_export_directive}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )

