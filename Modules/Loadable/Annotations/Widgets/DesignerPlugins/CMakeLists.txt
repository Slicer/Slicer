set(KIT ${PROJECT_NAME}Plugins)
project(${KIT})

#
# See CTK/CMake/ctkMacroBuildQtPlugin.cmake for details
#
# The header '${KIT}Export.h' will be automatically configured.
set(${KIT}_EXPORT "Q_SLICER_MODULE_ANNOTATIONS_WIDGETS_PLUGINS_EXPORT")

# Source files
set(${KIT}_SRCS
  qSlicerAnnotationModuleWidgetsAbstractPlugin.cxx
  qSlicerAnnotationModuleWidgetsAbstractPlugin.h
  qSlicerAnnotationModuleWidgetsPlugin.cxx
  qSlicerAnnotationModuleWidgetsPlugin.h

  qMRMLAnnotationROIWidgetPlugin.cxx
  qMRMLAnnotationROIWidgetPlugin.h
  )


# Headers that should run through moc
set(${KIT}_MOC_SRCS
  qSlicerAnnotationModuleWidgetsPlugin.h

  qMRMLAnnotationROIWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerAnnotationsModuleWidgets
  )

ctkMacroBuildQtDesignerPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

