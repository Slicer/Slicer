project(qSlicer${qt_module_name}ModuleWidgetsPlugin)

#
# See CMake/qCTKDesignerPluginMacros.cmake for details
#

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
  qSlicerAnnotationModuleWidgets
  )

qctk_build_designer_plugin(
  NAME ${PROJECT_NAME}
  SRCS ${designer_plugin_SRCS}
  MOC_SRCS ${designer_plugin_MOC_SRCS}
  TARGET_LIBRARIES ${designer_plugin_TARGET_LIBRARIES}
  )

