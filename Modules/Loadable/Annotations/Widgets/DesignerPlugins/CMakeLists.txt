
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicerAnnotationModuleWidgetsAbstractPlugin.cxx
  qSlicerAnnotationModuleWidgetsAbstractPlugin.h
  qSlicerAnnotationModuleWidgetsPlugin.cxx
  qSlicerAnnotationModuleWidgetsPlugin.h

  qMRMLAnnotationROIWidgetPlugin.cxx
  qMRMLAnnotationROIWidgetPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicerAnnotationModuleWidgetsPlugin.h

  qMRMLAnnotationROIWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerAnnotationsModuleWidgets
  )

#-----------------------------------------------------------------------------
ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
